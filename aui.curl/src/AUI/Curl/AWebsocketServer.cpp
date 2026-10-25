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

#include "AWebsocketServer.h"

#include <algorithm>
#include <atomic>
#include <list>
#include <mutex>
#include <thread>

#include "AUI/Crypt/AHash.h"
#include "AUI/Curl/Detail/WebsocketFrame.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Network/AInet4Address.h"
#include "AUI/Network/ATcpServerSocket.h"
#include "AUI/Network/ATcpSocket.h"

using namespace aui::websocket;

namespace {

constexpr auto LOG_TAG = "AWebsocketServer";
constexpr std::string_view HANDSHAKE_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

std::string lowercased(std::string_view s) {
    std::string r(s);
    std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return std::tolower(c); });
    return r;
}

std::string_view trimmed(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) s.remove_suffix(1);
    return s;
}

/// Whether a comma separated header value (i.e. `keep-alive, Upgrade`) contains the token.
bool hasToken(std::string_view value, std::string_view token) {
    while (!value.empty()) {
        auto comma = value.find(',');
        if (lowercased(trimmed(value.substr(0, comma))) == token) {
            return true;
        }
        if (comma == std::string_view::npos) {
            break;
        }
        value.remove_prefix(comma + 1);
    }
    return false;
}

struct ParsedRequest {
    AWebsocketServer::Request request;
    std::string key;
};

}   // namespace

AOptional<AString> AWebsocketServer::Request::queryParameter(const AString& name) const {
    std::string_view t = target.toStdString();
    auto q = t.find('?');
    if (q == std::string_view::npos) {
        return std::nullopt;
    }
    t.remove_prefix(q + 1);
    while (!t.empty()) {
        auto amp = t.find('&');
        auto pair = t.substr(0, amp);
        auto eq = pair.find('=');
        if (pair.substr(0, eq) == name.toStdString()) {
            return AString(eq == std::string_view::npos ? std::string_view {} : pair.substr(eq + 1));
        }
        if (amp == std::string_view::npos) {
            break;
        }
        t.remove_prefix(amp + 1);
    }
    return std::nullopt;
}

struct AWebsocketServer::Impl {
    class ConnectionImpl;

    Config config;
    _<ATcpServerSocket> serverSocket;
    std::uint16_t port = 0;
    std::thread acceptor;
    std::atomic<bool> stopping = false;
    bool started = false;

    std::mutex connectionsMutex;
    std::list<_<ConnectionImpl>> connections;

    void acceptLoop();
    void reapFinished();
};

class AWebsocketServer::Impl::ConnectionImpl : public AWebsocketServer::Connection,
                                               public std::enable_shared_from_this<ConnectionImpl> {
public:
    ConnectionImpl(Impl& server, _<ATcpSocket> socket) : mServer(server), mSocket(std::move(socket)) {}

    ~ConnectionImpl() override { join(); }

    void startThread() {
        // the thread keeps the connection alive until the loop is over.
        mThread = std::thread([self = shared_from_this()] { self->run(); });
    }

    void join() {
        if (!mThread.joinable()) {
            return;
        }
        if (mThread.get_id() == std::this_thread::get_id()) {
            // the last reference was released by the connection's own thread; it is about to exit anyway.
            mThread.detach();
            return;
        }
        mThread.join();
    }

    [[nodiscard]] bool finished() const noexcept { return mFinished; }

    const Request& request() const override { return mRequest; }

    bool sendText(AStringView text) override {
        return send(Opcode::TEXT, AByteBufferView(text.data(), text.size()));
    }

    bool sendBinary(AByteBufferView data) override { return send(Opcode::BINARY, data); }

    void close(std::uint16_t code) override {
        sendClose(code);
        shutdownSocket();
    }

    bool isOpen() const override { return mOpen; }

    /// Wakes up the reader thread.
    void shutdownSocket() {
        std::lock_guard lock(mWriteMutex);
        mOpen = false;
        if (mSocket) {
            mSocket->close();
        }
    }

private:
    Impl& mServer;
    _<ATcpSocket> mSocket;
    Request mRequest;
    std::thread mThread;
    std::mutex mWriteMutex;
    std::atomic<bool> mOpen = false;
    std::atomic<bool> mFinished = false;
    bool mCloseSent = false;

    bool writeLocked(AByteBufferView data) {
        std::lock_guard lock(mWriteMutex);
        if (!mSocket || (!mOpen && !mHandshaking)) {
            return false;
        }
        try {
            mSocket->write(data.data(), data.size());
            return true;
        } catch (const AException& e) {
            ALogger::debug(LOG_TAG) << "write failed: " << e.getMessage();
            return false;
        }
    }

    bool send(Opcode opcode, AByteBufferView payload) {
        if (!mOpen) {
            return false;
        }
        return writeLocked(encodeFrame(opcode, payload));
    }

    void sendClose(std::uint16_t code) {
        {
            std::lock_guard lock(mWriteMutex);
            if (mCloseSent) {
                return;
            }
            mCloseSent = true;
        }
        const char payload[2] = { static_cast<char>(code >> 8), static_cast<char>(code & 0xff) };
        writeLocked(encodeFrame(Opcode::CLOSE, AByteBufferView(payload, 2)));
    }

    bool mHandshaking = true;

    void respond(std::string_view status, std::string_view extraHeaders = {}) {
        std::string response = "HTTP/1.1 ";
        response += status;
        response += "\r\nConnection: close\r\nContent-Length: 0\r\n";
        response += extraHeaders;
        response += "\r\n";
        writeLocked(AByteBufferView(response.data(), response.size()));
    }

    /// Reads the HTTP request head. Returns nothing on EOF / oversize.
    AOptional<std::string> readRequestHead() {
        std::string head;
        char chunk[1024];
        while (head.find("\r\n\r\n") == std::string::npos) {
            if (head.size() > mServer.config.maxHeaderSize) {
                respond("431 Request Header Fields Too Large");
                return std::nullopt;
            }
            auto n = mSocket->read(chunk, sizeof(chunk));
            if (n == 0) {
                return std::nullopt;
            }
            head.append(chunk, n);
        }
        return head;
    }

    bool parseRequest(const std::string& head, ParsedRequest& out) {
        std::string_view v = head;
        v = v.substr(0, v.find("\r\n\r\n"));

        auto lineEnd = v.find("\r\n");
        auto requestLine = v.substr(0, lineEnd);
        auto sp1 = requestLine.find(' ');
        auto sp2 = requestLine.rfind(' ');
        if (sp1 == std::string_view::npos || sp2 == sp1) {
            return false;
        }
        out.request.method = AString(requestLine.substr(0, sp1));
        out.request.target = AString(requestLine.substr(sp1 + 1, sp2 - sp1 - 1));
        if (requestLine.substr(sp2 + 1) != "HTTP/1.1") {
            return false;
        }

        while (lineEnd != std::string_view::npos) {
            v.remove_prefix(lineEnd + 2);
            lineEnd = v.find("\r\n");
            auto line = v.substr(0, lineEnd);
            if (line.empty()) {
                continue;
            }
            auto colon = line.find(':');
            if (colon == std::string_view::npos) {
                return false;
            }
            auto name = lowercased(trimmed(line.substr(0, colon)));
            auto value = std::string(trimmed(line.substr(colon + 1)));
            auto& slot = out.request.headers[AString(name)];
            slot = slot.empty() ? AString(value) : slot + ", " + AString(value);
        }
        return true;
    }

    bool performHandshake() {
        mSocket->setTimeout(mServer.config.handshakeTimeout);
        auto head = readRequestHead();
        if (!head) {
            return false;
        }
        ParsedRequest parsed;
        if (!parseRequest(*head, parsed)) {
            respond("400 Bad Request");
            return false;
        }
        mRequest = std::move(parsed.request);

        auto header = [&](const char* name) -> std::string {
            auto it = mRequest.headers.find(name);
            return it == mRequest.headers.end() ? std::string {} : it->second.toStdString();
        };

        if (mRequest.method != "GET" || lowercased(header("upgrade")) != "websocket" ||
            !hasToken(header("connection"), "upgrade")) {
            respond("400 Bad Request");
            return false;
        }
        if (header("sec-websocket-version") != "13") {
            respond("426 Upgrade Required", "Sec-WebSocket-Version: 13\r\n");
            return false;
        }
        const auto key = header("sec-websocket-key");
        try {
            if (AByteBuffer::fromBase64String(AString(key)).size() != 16) {
                respond("400 Bad Request");
                return false;
            }
        } catch (const AException&) {
            respond("400 Bad Request");
            return false;
        }

        if (mServer.config.verifyHandshake && !mServer.config.verifyHandshake(mRequest)) {
            respond("403 Forbidden");
            return false;
        }

        auto accept = AHash::sha1(AByteBuffer::fromString(AString(key + std::string(HANDSHAKE_GUID)))).toBase64String();
        std::string response =
            "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " +
            accept.toStdString() + "\r\n";
        // a client that offered subprotocols expects one of them back (AWebsocket always offers "chat").
        if (auto protocols = header("sec-websocket-protocol"); !protocols.empty()) {
            response += "Sec-WebSocket-Protocol: " + std::string(trimmed(protocols.substr(0, protocols.find(',')))) + "\r\n";
        }
        response += "\r\n";
        mOpen = true;
        if (!writeLocked(AByteBufferView(response.data(), response.size()))) {
            return false;
        }
        mHandshaking = false;
        mSocket->setTimeout(std::chrono::milliseconds(0));
        return true;
    }

    void deliver(AByteBuffer message, bool binary) {
        if (onMessage) {
            try {
                onMessage(std::move(message), binary);
            } catch (const AException& e) {
                ALogger::err(LOG_TAG) << "exception in onMessage: " << e.getMessage();
            } catch (const std::exception& e) {
                ALogger::err(LOG_TAG) << "exception in onMessage: " << e.what();
            }
        }
    }

    void readLoop() {
        FrameDecoder decoder(true, mServer.config.maxMessageSize);
        AByteBuffer assembling;
        bool assemblingBinary = false;
        bool assemblingActive = false;

        char chunk[64 * 1024];
        try {
            for (;;) {
                const auto n = mSocket->read(chunk, sizeof(chunk));
                if (n == 0) {
                    return;   // EOF
                }
                decoder.feed(AByteBufferView(chunk, n));
                while (auto frame = decoder.next()) {
                    switch (frame->opcode) {
                        case Opcode::TEXT:
                        case Opcode::BINARY:
                            if (assemblingActive) {
                                throw ProtocolError(close_code::PROTOCOL_ERROR, "new message inside a fragmented one");
                            }
                            if (frame->fin) {
                                deliver(std::move(frame->payload), frame->opcode == Opcode::BINARY);
                            } else {
                                assembling = std::move(frame->payload);
                                assemblingBinary = frame->opcode == Opcode::BINARY;
                                assemblingActive = true;
                            }
                            break;
                        case Opcode::CONTINUATION:
                            if (!assemblingActive) {
                                throw ProtocolError(close_code::PROTOCOL_ERROR, "unexpected continuation frame");
                            }
                            if (assembling.size() + frame->payload.size() > mServer.config.maxMessageSize) {
                                throw ProtocolError(close_code::MESSAGE_TOO_BIG, "message is too big");
                            }
                            assembling.write(frame->payload.data(), frame->payload.size());
                            if (frame->fin) {
                                assemblingActive = false;
                                deliver(std::move(assembling), assemblingBinary);
                                assembling = {};
                            }
                            break;
                        case Opcode::PING:
                            writeLocked(encodeFrame(Opcode::PONG, frame->payload));
                            break;
                        case Opcode::PONG:
                            break;
                        case Opcode::CLOSE: {
                            std::uint16_t code = close_code::NORMAL;
                            if (frame->payload.size() >= 2) {
                                code = static_cast<std::uint16_t>(
                                    (static_cast<std::uint8_t>(frame->payload.data()[0]) << 8) |
                                    static_cast<std::uint8_t>(frame->payload.data()[1]));
                            }
                            sendClose(code);
                            return;
                        }
                    }
                }
            }
        } catch (const ProtocolError& e) {
            ALogger::debug(LOG_TAG) << "protocol error: " << e.what();
            sendClose(e.closeCode);
        } catch (const AException& e) {
            // connection broken, or the socket was closed by stop().
            ALogger::debug(LOG_TAG) << "read failed: " << e.getMessage();
        }
    }

    /// Closing a socket that has unread data makes the system send RST, which may destroy the response the client has
    /// not read yet. So after an error response, let the client finish sending and leave.
    void drainInput() {
        try {
            mSocket->setTimeout(std::chrono::milliseconds(300));
            char chunk[1024];
            while (mSocket->read(chunk, sizeof(chunk)) > 0) {
            }
        } catch (const AException&) {
        }
    }

    void run() {
        try {
            if (performHandshake()) {
                if (mServer.config.onConnected) {
                    mServer.config.onConnected(shared_from_this());
                }
                readLoop();
            } else {
                drainInput();
            }
        } catch (const AException& e) {
            ALogger::debug(LOG_TAG) << "connection failed: " << e.getMessage();
        } catch (const std::exception& e) {
            ALogger::err(LOG_TAG) << "connection failed: " << e.what();
        }
        shutdownSocket();
        if (onClosed) {
            try {
                onClosed();
            } catch (const std::exception& e) {
                ALogger::err(LOG_TAG) << "exception in onClosed: " << e.what();
            }
        }
        // break the reference cycles users typically create by capturing the connection in the callbacks.
        onMessage = nullptr;
        onClosed = nullptr;
        mFinished = true;
    }
};

void AWebsocketServer::Impl::reapFinished() {
    std::list<_<ConnectionImpl>> finished;
    {
        std::lock_guard lock(connectionsMutex);
        for (auto it = connections.begin(); it != connections.end();) {
            if ((*it)->finished()) {
                finished.splice(finished.end(), connections, it++);
            } else {
                ++it;
            }
        }
    }
    for (auto& c : finished) {
        c->join();
    }
}

void AWebsocketServer::Impl::acceptLoop() {
    while (!stopping) {
        _<ATcpSocket> client;
        try {
            client = serverSocket->accept();
        } catch (const AException& e) {
            if (stopping) {
                return;
            }
            ALogger::err(LOG_TAG) << "accept failed: " << e.getMessage();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        if (stopping) {
            return;   // the wake-up connection from stop()
        }
        reapFinished();
        auto connection = aui::ptr::manage_shared(new ConnectionImpl(*this, std::move(client)));
        {
            std::lock_guard lock(connectionsMutex);
            connections.push_back(connection);
        }
        connection->startThread();
    }
}

AWebsocketServer::AWebsocketServer(Config config) : mImpl(std::make_unique<Impl>()) {
    mImpl->config = std::move(config);
    mImpl->serverSocket = aui::ptr::manage_shared(
        new ATcpServerSocket(AInet4Address::fromString(mImpl->config.bindAddress, mImpl->config.port)));
    mImpl->port = mImpl->serverSocket->getAddress().port();
}

AWebsocketServer::~AWebsocketServer() { stop(); }

void AWebsocketServer::start() {
    if (mImpl->started) {
        return;
    }
    mImpl->started = true;
    mImpl->acceptor = std::thread([this] { mImpl->acceptLoop(); });
}

void AWebsocketServer::stop() {
    if (!mImpl || mImpl->stopping.exchange(true)) {
        return;
    }
    if (mImpl->started) {
        // accept() is blocking; shutdown() of a listening socket doesn't wake it on every platform, so connect to it.
        try {
            ATcpSocket wake(AInet4Address::fromString(mImpl->config.bindAddress == "0.0.0.0" ? "127.0.0.1" : mImpl->config.bindAddress,
                                                      mImpl->port));
        } catch (const AException&) {
        }
        if (mImpl->acceptor.joinable()) {
            mImpl->acceptor.join();
        }
    }
    mImpl->serverSocket->close();

    std::list<_<Impl::ConnectionImpl>> connections;
    {
        std::lock_guard lock(mImpl->connectionsMutex);
        connections = std::move(mImpl->connections);
    }
    for (auto& c : connections) {
        c->shutdownSocket();
    }
    for (auto& c : connections) {
        c->join();
    }
}

std::uint16_t AWebsocketServer::port() const noexcept { return mImpl->port; }
