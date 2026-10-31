/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <thread>

#include "AUI/Crypt/ATlsSession.h"
#include "AUI/Curl/AWebsocket.h"
#include "AUI/Curl/AWebsocketServer.h"
#include "AUI/Curl/Detail/WebsocketFrame.h"
#include "AUI/IO/AEOFException.h"
#include "AUI/Network/AInet4Address.h"
#include "AUI/Network/ATcpSocket.h"
#include "AUI/Thread/AThread.h"

using namespace aui::websocket;
using namespace std::chrono_literals;

namespace {

constexpr auto SAMPLE_KEY = "dGhlIHNhbXBsZSBub25jZQ==";   // RFC 6455, section 1.3
constexpr auto SAMPLE_ACCEPT = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";
constexpr Mask MASK = {0xa1, 0xb2, 0xc3, 0xd4};

AByteBuffer text(std::string_view s) { return AByteBuffer(s.data(), s.size()); }

/// Echoes every message back.
AWebsocketServer::Config echoConfig() {
    AWebsocketServer::Config config;
    config.onConnected = [](_<AWebsocketServer::Connection> connection) {
        // the connection is kept alive by the server, so a raw pointer is enough for the callback.
        auto* c = connection.get();
        connection->onMessage = [c](AByteBuffer message, bool binary) {
            if (binary) {
                c->sendBinary(message);
            } else {
                c->sendText(AStringView(message.data(), message.size()));
            }
        };
    };
    return config;
}

/// Minimal client speaking raw bytes, so the tests can send malformed things.
class RawClient {
public:
    explicit RawClient(std::uint16_t port)
      : mSocket(AInet4Address::fromString("127.0.0.1", port)), mDecoder(false, 1 << 28) {
        mSocket.setTimeout(5s);
    }

    /// Speaks TLS; `pin` is the fingerprint the server must present.
    RawClient(std::uint16_t port, AByteBuffer pin) : RawClient(port) {
        ATlsTransport transport {
            [this](std::chrono::milliseconds t) { return mSocket.waitForRead(t); },
            [this](std::span<std::byte> dst) { return mSocket.read(dst); },
            [this](std::span<const std::byte> src) {
                mSocket.write(reinterpret_cast<const char*>(src.data()), src.size());
            },
        };
        ATlsSession::ClientOptions options;
        options.expectedFingerprint = std::move(pin);
        mTls = ATlsSession::connect(std::move(transport), options, 5s);
    }

    void sendRaw(std::string_view data) { writeSome(data.data(), data.size()); }
    void sendRaw(const AByteBuffer& data) { writeSome(data.data(), data.size()); }

    static std::string upgradeRequest(std::string_view target = "/", std::string_view key = SAMPLE_KEY,
                                      std::string_view extra = {}) {
        std::string r = "GET ";
        r += target;
        r += " HTTP/1.1\r\nHost: 127.0.0.1\r\nUpgrade: websocket\r\nConnection: keep-alive, Upgrade\r\n"
             "Sec-WebSocket-Version: 13\r\nSec-WebSocket-Key: ";
        r += key;
        r += "\r\n";
        r += extra;
        r += "\r\n";
        return r;
    }

    /// Reads the HTTP response head.
    std::string readHead() {
        std::string head;
        char c;
        while (head.find("\r\n\r\n") == std::string::npos) {
            if (readSome(&c, 1) == 0) {
                break;
            }
            head += c;
        }
        return head;
    }

    std::string handshake(std::string_view target = "/") {
        sendRaw(upgradeRequest(target));
        return readHead();
    }

    void send(Opcode opcode, AByteBufferView payload, bool fin = true) {
        sendRaw(encodeFrame(opcode, payload, fin, MASK));
    }

    /// @return nothing on EOF
    AOptional<Frame> readFrame() {
        for (;;) {
            if (auto f = mDecoder.next()) {
                return f;
            }
            char buf[4096];
            auto n = readSome(buf, sizeof(buf));
            if (n == 0) {
                return std::nullopt;
            }
            mDecoder.feed(AByteBufferView(buf, n));
        }
    }

    static std::uint16_t closeCodeOf(const Frame& f) {
        EXPECT_GE(f.payload.size(), 2u);
        return static_cast<std::uint16_t>((static_cast<std::uint8_t>(f.payload.data()[0]) << 8) |
                                          static_cast<std::uint8_t>(f.payload.data()[1]));
    }

private:
    ATcpSocket mSocket;
    _<ATlsSession> mTls;
    FrameDecoder mDecoder;

    std::size_t readSome(char* dst, std::size_t size) {
        try {
            return mTls ? mTls->read(std::as_writable_bytes(std::span(dst, size))) : mSocket.read(dst, size);
        } catch (const AEOFException&) {
            return 0;   // the server reset the connection (Windows reports RST where Linux sees a clean FIN)
        }
    }
    void writeSome(const char* src, std::size_t size) {
        if (mTls) {
            mTls->write(std::as_bytes(std::span(src, size)));
        } else {
            mSocket.write(src, size);
        }
    }
};

bool startsWith(const std::string& s, std::string_view prefix) { return s.compare(0, prefix.size(), prefix) == 0; }

}   // namespace

TEST(WebsocketServerTest, PicksFreePort) {
    AWebsocketServer a(echoConfig());
    AWebsocketServer b(echoConfig());
    EXPECT_NE(a.port(), 0);
    EXPECT_NE(b.port(), 0);
    EXPECT_NE(a.port(), b.port());
}

TEST(WebsocketServerTest, Handshake) {
    AWebsocketServer server(echoConfig());
    server.start();
    RawClient client(server.port());
    auto head = client.handshake();
    EXPECT_TRUE(startsWith(head, "HTTP/1.1 101")) << head;
    EXPECT_NE(head.find(std::string("Sec-WebSocket-Accept: ") + SAMPLE_ACCEPT), std::string::npos) << head;
}

TEST(WebsocketServerTest, HandshakeEchoesFirstSubprotocol) {
    AWebsocketServer server(echoConfig());
    server.start();
    RawClient client(server.port());
    client.sendRaw(RawClient::upgradeRequest("/", SAMPLE_KEY, "Sec-WebSocket-Protocol: chat, superchat\r\n"));
    auto head = client.readHead();
    EXPECT_TRUE(startsWith(head, "HTTP/1.1 101")) << head;
    EXPECT_NE(head.find("Sec-WebSocket-Protocol: chat\r\n"), std::string::npos) << head;
}

TEST(WebsocketServerTest, BadHandshakes) {
    AWebsocketServer server(echoConfig());
    server.start();

    auto expectStatus = [&](std::string request, std::string_view status) {
        RawClient client(server.port());
        client.sendRaw(request);
        auto head = client.readHead();
        EXPECT_TRUE(startsWith(head, status)) << "request:\n" << request << "\nresponse:\n" << head;
    };

    // not an upgrade
    expectStatus("GET / HTTP/1.1\r\nHost: x\r\n\r\n", "HTTP/1.1 400");
    // wrong method
    {
        auto r = RawClient::upgradeRequest();
        r.replace(0, 3, "PUT");
        expectStatus(r, "HTTP/1.1 400");
    }
    // wrong version
    {
        auto r = RawClient::upgradeRequest();
        r.replace(r.find("Version: 13"), 11, "Version: 8 ");
        expectStatus(r, "HTTP/1.1 426");
    }
    // key is not 16 bytes
    expectStatus(RawClient::upgradeRequest("/", "YWJj"), "HTTP/1.1 400");
    // not http
    expectStatus("hello\r\n\r\n", "HTTP/1.1 400");
}

TEST(WebsocketServerTest, OversizedHeadersAreRejected) {
    auto config = echoConfig();
    config.maxHeaderSize = 512;
    AWebsocketServer server(std::move(config));
    server.start();
    RawClient client(server.port());
    client.sendRaw(RawClient::upgradeRequest("/", SAMPLE_KEY, "X-Padding: " + std::string(4096, 'a') + "\r\n"));
    EXPECT_TRUE(startsWith(client.readHead(), "HTTP/1.1 431"));
}

TEST(WebsocketServerTest, VerifyHandshakeCanReject) {
    auto config = echoConfig();
    config.verifyHandshake = [](const AWebsocketServer::Request& request) {
        auto token = request.queryParameter("token");
        return token && *token == "secret";
    };
    AWebsocketServer server(std::move(config));
    server.start();

    {
        RawClient client(server.port());
        EXPECT_TRUE(startsWith(client.handshake("/"), "HTTP/1.1 403"));
    }
    {
        RawClient client(server.port());
        EXPECT_TRUE(startsWith(client.handshake("/?a=b&token=wrong"), "HTTP/1.1 403"));
    }
    {
        RawClient client(server.port());
        EXPECT_TRUE(startsWith(client.handshake("/?a=b&token=secret"), "HTTP/1.1 101"));
    }
}

TEST(WebsocketServerTest, VerifyHandshakeSeesHeaders) {
    auto config = echoConfig();
    config.verifyHandshake = [](const AWebsocketServer::Request& request) {
        auto it = request.headers.find("origin");
        return it == request.headers.end();   // reject browsers
    };
    AWebsocketServer server(std::move(config));
    server.start();
    {
        RawClient client(server.port());
        client.sendRaw(RawClient::upgradeRequest("/", SAMPLE_KEY, "Origin: https://evil.example\r\n"));
        EXPECT_TRUE(startsWith(client.readHead(), "HTTP/1.1 403"));
    }
    {
        RawClient client(server.port());
        EXPECT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));
    }
}

TEST(WebsocketServerTest, EchoText) {
    AWebsocketServer server(echoConfig());
    server.start();
    RawClient client(server.port());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));

    client.send(Opcode::TEXT, text("hello"));
    auto reply = client.readFrame();
    ASSERT_TRUE(reply);
    EXPECT_EQ(reply->opcode, Opcode::TEXT);
    EXPECT_EQ(reply->payload, text("hello"));
}

TEST(WebsocketServerTest, EchoPayloadBoundaries) {
    AWebsocketServer server(echoConfig());
    server.start();
    RawClient client(server.port());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));

    for (std::size_t size : {0u, 125u, 126u, 65535u, 65536u, 300000u}) {
        AByteBuffer payload;
        payload.resize(size);
        for (std::size_t i = 0; i < size; ++i) {
            payload.data()[i] = static_cast<char>(i * 7);
        }
        client.send(Opcode::BINARY, payload);
        auto reply = client.readFrame();
        ASSERT_TRUE(reply) << "size=" << size;
        EXPECT_EQ(reply->opcode, Opcode::BINARY);
        EXPECT_EQ(reply->payload, payload) << "size=" << size;
    }
}

TEST(WebsocketServerTest, FragmentedMessageIsReassembled) {
    AWebsocketServer server(echoConfig());
    server.start();
    RawClient client(server.port());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));

    client.send(Opcode::TEXT, text("Hel"), false);
    client.send(Opcode::PING, text("p"));   // control frames may be interleaved
    client.send(Opcode::CONTINUATION, text("lo, "), false);
    client.send(Opcode::CONTINUATION, text("world"), true);

    auto pong = client.readFrame();
    ASSERT_TRUE(pong);
    EXPECT_EQ(pong->opcode, Opcode::PONG);
    EXPECT_EQ(pong->payload, text("p"));

    auto reply = client.readFrame();
    ASSERT_TRUE(reply);
    EXPECT_EQ(reply->opcode, Opcode::TEXT);
    EXPECT_EQ(reply->payload, text("Hel" "lo, " "world"));
}

TEST(WebsocketServerTest, PingGetsPongWithSamePayload) {
    AWebsocketServer server(echoConfig());
    server.start();
    RawClient client(server.port());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));

    client.send(Opcode::PING, text("are you there"));
    auto pong = client.readFrame();
    ASSERT_TRUE(pong);
    EXPECT_EQ(pong->opcode, Opcode::PONG);
    EXPECT_EQ(pong->payload, text("are you there"));
}

TEST(WebsocketServerTest, CloseHandshake) {
    std::atomic<bool> closed = false;
    auto config = echoConfig();
    auto original = config.onConnected;
    config.onConnected = [&closed, original](_<AWebsocketServer::Connection> c) {
        original(c);
        c->onClosed = [&closed] { closed = true; };
    };
    AWebsocketServer server(std::move(config));
    server.start();
    RawClient client(server.port());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));

    const char payload[] = { 0x03, static_cast<char>(0xe8) };   // 1000
    client.send(Opcode::CLOSE, AByteBufferView(payload, 2));
    auto reply = client.readFrame();
    ASSERT_TRUE(reply);
    EXPECT_EQ(reply->opcode, Opcode::CLOSE);
    EXPECT_EQ(RawClient::closeCodeOf(*reply), close_code::NORMAL);
    EXPECT_FALSE(client.readFrame());   // EOF
    for (int i = 0; i < 100 && !closed; ++i) {
        std::this_thread::sleep_for(10ms);
    }
    EXPECT_TRUE(closed);
}

TEST(WebsocketServerTest, UnmaskedClientFrameIsProtocolError) {
    AWebsocketServer server(echoConfig());
    server.start();
    RawClient client(server.port());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));

    client.sendRaw(encodeFrame(Opcode::TEXT, text("x")));   // no mask
    auto reply = client.readFrame();
    ASSERT_TRUE(reply);
    EXPECT_EQ(reply->opcode, Opcode::CLOSE);
    EXPECT_EQ(RawClient::closeCodeOf(*reply), close_code::PROTOCOL_ERROR);
}

TEST(WebsocketServerTest, ContinuationWithoutStartIsProtocolError) {
    AWebsocketServer server(echoConfig());
    server.start();
    RawClient client(server.port());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));

    client.send(Opcode::CONTINUATION, text("x"));
    auto reply = client.readFrame();
    ASSERT_TRUE(reply);
    EXPECT_EQ(reply->opcode, Opcode::CLOSE);
    EXPECT_EQ(RawClient::closeCodeOf(*reply), close_code::PROTOCOL_ERROR);
}

TEST(WebsocketServerTest, OversizedMessageClosesWith1009) {
    auto config = echoConfig();
    config.maxMessageSize = 1000;
    AWebsocketServer server(std::move(config));
    server.start();
    RawClient client(server.port());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));

    // a single big frame
    client.send(Opcode::BINARY, AByteBuffer(std::string(2000, 'x').data(), 2000));
    auto reply = client.readFrame();
    ASSERT_TRUE(reply);
    EXPECT_EQ(reply->opcode, Opcode::CLOSE);
    EXPECT_EQ(RawClient::closeCodeOf(*reply), close_code::MESSAGE_TOO_BIG);
}

TEST(WebsocketServerTest, OversizedFragmentedMessageClosesWith1009) {
    auto config = echoConfig();
    config.maxMessageSize = 1000;
    AWebsocketServer server(std::move(config));
    server.start();
    RawClient client(server.port());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));

    auto chunk = AByteBuffer(std::string(600, 'x').data(), 600);
    client.send(Opcode::BINARY, chunk, false);
    client.send(Opcode::CONTINUATION, chunk, true);
    auto reply = client.readFrame();
    ASSERT_TRUE(reply);
    EXPECT_EQ(reply->opcode, Opcode::CLOSE);
    EXPECT_EQ(RawClient::closeCodeOf(*reply), close_code::MESSAGE_TOO_BIG);
}

TEST(WebsocketServerTest, ServerCanSendOnItsOwn) {
    std::atomic<AWebsocketServer::Connection*> connection = nullptr;
    AWebsocketServer::Config config;
    config.onConnected = [&](_<AWebsocketServer::Connection> c) {
        c->sendText("welcome");
        connection = c.get();
    };
    AWebsocketServer server(std::move(config));
    server.start();
    RawClient client(server.port());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));
    auto first = client.readFrame();
    ASSERT_TRUE(first);
    EXPECT_EQ(first->payload, text("welcome"));

    // from another thread, later
    for (int i = 0; i < 100 && !connection; ++i) {
        std::this_thread::sleep_for(10ms);
    }
    ASSERT_TRUE(connection);
    std::thread([&] { connection.load()->sendText("later"); }).join();
    auto second = client.readFrame();
    ASSERT_TRUE(second);
    EXPECT_EQ(second->payload, text("later"));
}

TEST(WebsocketServerTest, ServerCloseSendsCloseFrame) {
    AWebsocketServer::Config config;
    config.onConnected = [](_<AWebsocketServer::Connection> c) { c->close(1001); };
    AWebsocketServer server(std::move(config));
    server.start();
    RawClient client(server.port());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));
    auto reply = client.readFrame();
    ASSERT_TRUE(reply);
    EXPECT_EQ(reply->opcode, Opcode::CLOSE);
    EXPECT_EQ(RawClient::closeCodeOf(*reply), 1001);
}

TEST(WebsocketServerTest, ConcurrentClients) {
    AWebsocketServer server(echoConfig());
    server.start();

    constexpr int CLIENTS = 8;
    std::atomic<int> ok = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < CLIENTS; ++i) {
        threads.emplace_back([&, i] {
            RawClient client(server.port());
            if (!startsWith(client.handshake(), "HTTP/1.1 101")) {
                return;
            }
            for (int j = 0; j < 20; ++j) {
                auto msg = text("client " + std::to_string(i) + " message " + std::to_string(j));
                client.send(Opcode::TEXT, msg);
                auto reply = client.readFrame();
                if (!reply || reply->payload != msg) {
                    return;
                }
            }
            ++ok;
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    EXPECT_EQ(ok, CLIENTS);
}

TEST(WebsocketServerTest, StopWithConnectedClients) {
    auto server = std::make_unique<AWebsocketServer>(echoConfig());
    server->start();
    RawClient connected(server->port());
    ASSERT_TRUE(startsWith(connected.handshake(), "HTTP/1.1 101"));
    RawClient notHandshaked(server->port());   // stuck in the handshake stage

    server.reset();   // must neither hang nor crash

    EXPECT_FALSE(connected.readFrame());   // connection dropped
}

TEST(WebsocketServerTest, StopWithoutStart) {
    AWebsocketServer server(echoConfig());
    server.stop();
}

TEST(WebsocketServerTest, StopTwiceAndRestartOnTheSamePort) {
    std::uint16_t port;
    {
        AWebsocketServer server(echoConfig());
        server.start();
        port = server.port();
        server.stop();
        server.stop();
    }
    auto config = echoConfig();
    config.port = port;
    AWebsocketServer again(std::move(config));
    again.start();
    EXPECT_EQ(again.port(), port);
}

TEST(WebsocketServerTest, ClientDisconnectInTheMiddleOfAFrame) {
    AWebsocketServer server(echoConfig());
    server.start();
    {
        RawClient client(server.port());
        ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));
        auto frame = encodeFrame(Opcode::TEXT, text("truncated"), true, MASK);
        client.sendRaw(std::string_view(frame.data(), frame.size() - 3));
    }   // closed here
    // the server is still alive
    RawClient another(server.port());
    EXPECT_TRUE(startsWith(another.handshake(), "HTTP/1.1 101"));
}

TEST(WebsocketServerTest, AWebsocketClientEcho) {
    AWebsocketServer server(echoConfig());
    server.start();

    auto ws = _new<AWebsocket>("ws://127.0.0.1:" + AString::number(server.port()) + "/");
    std::string received;
    AObject::connect(ws->connected, ws, [&] { ws->writeText("hello from AWebsocket"); });
    AObject::connect(ws->received, ws, [&](AByteBufferView payload) {
        received += std::string_view(payload.data(), payload.size());
        ws->close();
    });
    ws->run();
    EXPECT_EQ(received, "hello from AWebsocket");
}

namespace {
AWebsocketServer::Config tlsEchoConfig(_<ATlsIdentity> identity) {
    auto config = echoConfig();
    config.tls = std::move(identity);
    return config;
}
}   // namespace

TEST(WebsocketServerTlsTest, EchoOverTls) {
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    AWebsocketServer server(tlsEchoConfig(identity));
    server.start();
    RawClient client(server.port(), identity->fingerprintSha256());
    ASSERT_TRUE(startsWith(client.handshake(), "HTTP/1.1 101"));
    for (std::size_t size : {0u, 125u, 126u, 65536u, 300000u}) {
        AByteBuffer payload;
        payload.resize(size);
        for (std::size_t i = 0; i < size; ++i) {
            payload.data()[i] = static_cast<char>(i * 13);
        }
        client.send(Opcode::BINARY, payload);
        auto reply = client.readFrame();
        ASSERT_TRUE(reply) << "size=" << size;
        EXPECT_EQ(reply->payload, payload) << "size=" << size;
    }
}

TEST(WebsocketServerTlsTest, ClientWithWrongFingerprintGivesUp) {
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    auto other = ATlsIdentity::generateSelfSigned("aui-test");
    AWebsocketServer server(tlsEchoConfig(identity));
    server.start();
    EXPECT_THROW(RawClient(server.port(), other->fingerprintSha256()), ATlsException);
}

TEST(WebsocketServerTlsTest, PlainClientIsDropped) {
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    AWebsocketServer server(tlsEchoConfig(identity));
    server.start();
    RawClient client(server.port());
    client.sendRaw(RawClient::upgradeRequest());
    // no HTTP response: the server can't make sense of the bytes and hangs up
    EXPECT_FALSE(startsWith(client.readHead(), "HTTP/1.1"));
}

TEST(WebsocketServerTlsTest, TlsClientIsDroppedByPlainServer) {
    AWebsocketServer server(echoConfig());
    server.start();
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    EXPECT_ANY_THROW(RawClient(server.port(), identity->fingerprintSha256()));
}

TEST(WebsocketServerTlsTest, VerifyHandshakeStillApplies) {
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    auto config = tlsEchoConfig(identity);
    config.verifyHandshake = [](const AWebsocketServer::Request& request) {
        return bool(request.queryParameter("key"));
    };
    AWebsocketServer server(std::move(config));
    server.start();
    {
        RawClient client(server.port(), identity->fingerprintSha256());
        EXPECT_TRUE(startsWith(client.handshake("/"), "HTTP/1.1 403"));
    }
    {
        RawClient client(server.port(), identity->fingerprintSha256());
        EXPECT_TRUE(startsWith(client.handshake("/?key=x"), "HTTP/1.1 101"));
    }
}

TEST(WebsocketServerTlsTest, AWebsocketClientOverWss) {
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    AWebsocketServer server(tlsEchoConfig(identity));
    server.start();

    auto ws = _new<AWebsocket>("wss://127.0.0.1:" + AString::number(server.port()) + "/");
    std::string received;
    AObject::connect(ws->connected, ws, [&] { ws->writeText("hello over tls"); });
    AObject::connect(ws->received, ws, [&](AByteBufferView payload) {
        received += std::string_view(payload.data(), payload.size());
        ws->close();
    });
    ws->run();
    EXPECT_EQ(received, "hello over tls");
}

TEST(WebsocketServerTlsTest, PinnedCertificateIsAccepted) {
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    AWebsocketServer server(tlsEchoConfig(identity));
    server.start();

    AWebsocket ws("wss://127.0.0.1:" + AString::number(server.port()) + "/",
                  AWebsocket::Options { .expectedFingerprint = identity->fingerprintSha256() });
    ws.connect();
    ws.writeText("pinned");
    auto message = ws.read(std::chrono::seconds(5));
    ASSERT_TRUE(message.hasValue());
    EXPECT_EQ(AString::fromUtf8(*message), "pinned");
}

TEST(WebsocketServerTlsTest, OtherCertificateIsRejected) {
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    AWebsocketServer server(tlsEchoConfig(identity));
    server.start();

    AWebsocket ws("wss://127.0.0.1:" + AString::number(server.port()) + "/",
                  AWebsocket::Options {
                    .expectedFingerprint = ATlsIdentity::generateSelfSigned("other")->fingerprintSha256(),
                  });
    EXPECT_THROW(ws.connect(), ATlsException);
}

TEST(WebsocketServerTest, ReadTimesOutWithoutMessages) {
    AWebsocketServer server(echoConfig());
    server.start();

    AWebsocket ws("ws://127.0.0.1:" + AString::number(server.port()) + "/");
    ws.connect();
    EXPECT_FALSE(ws.read(std::chrono::milliseconds(50)).hasValue());
    ws.writeText("still alive");
    EXPECT_TRUE(ws.read(std::chrono::seconds(5)).hasValue());
}

TEST(WebsocketServerTlsTest, StopWithConnectedTlsClients) {
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    auto server = std::make_unique<AWebsocketServer>(tlsEchoConfig(identity));
    server->start();
    RawClient connected(server->port(), identity->fingerprintSha256());
    ASSERT_TRUE(startsWith(connected.handshake(), "HTTP/1.1 101"));
    server.reset();   // must neither hang nor crash
    EXPECT_ANY_THROW({
        if (!connected.readFrame()) {
            throw std::runtime_error("eof");
        }
    });
}
