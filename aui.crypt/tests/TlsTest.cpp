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

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include "AUI/Crypt/ATlsIdentity.h"
#include "AUI/Crypt/ATlsSession.h"

using namespace std::chrono_literals;

namespace {

std::span<const std::byte> bytes(std::string_view data) { return std::as_bytes(std::span(data)); }

/// One direction of an in-memory connection.
class Pipe {
public:
    void push(std::span<const std::byte> data) {
        {
            std::lock_guard lock(mMutex);
            for (std::byte c : data) {
                if (mCorruptNext && mPushed++ >= mCorruptFrom) {
                    c ^= std::byte(0x55);
                }
                mBytes.push_back(c);
            }
        }
        mCondition.notify_all();
    }

    void closeWriter() {
        {
            std::lock_guard lock(mMutex);
            mClosed = true;
        }
        mCondition.notify_all();
    }

    bool waitForRead(std::chrono::milliseconds timeout) {
        std::unique_lock lock(mMutex);
        return mCondition.wait_for(lock, timeout, [&] { return !mBytes.empty() || mClosed; });
    }

    std::size_t read(std::span<std::byte> dst) {
        std::unique_lock lock(mMutex);
        mCondition.wait(lock, [&] { return !mBytes.empty() || mClosed; });
        std::size_t n = 0;
        while (n < dst.size() && !mBytes.empty()) {
            dst[n++] = mBytes.front();
            mBytes.pop_front();
        }
        return n;
    }

    /// Flips bits of everything pushed after the first `from` bytes.
    void corruptAfter(std::size_t from) {
        std::lock_guard lock(mMutex);
        mCorruptNext = true;
        mCorruptFrom = from;
    }

private:
    std::mutex mMutex;
    std::condition_variable mCondition;
    std::deque<std::byte> mBytes;
    bool mClosed = false;
    bool mCorruptNext = false;
    std::size_t mCorruptFrom = 0;
    std::size_t mPushed = 0;
};

struct Link {
    Pipe clientToServer;
    Pipe serverToClient;

    ATlsTransport clientSide() {
        return {
            [this](std::chrono::milliseconds t) { return serverToClient.waitForRead(t); },
            [this](std::span<std::byte> dst) { return serverToClient.read(dst); },
            [this](std::span<const std::byte> src) { clientToServer.push(src); },
        };
    }

    ATlsTransport serverSide() {
        return {
            [this](std::chrono::milliseconds t) { return clientToServer.waitForRead(t); },
            [this](std::span<std::byte> dst) { return clientToServer.read(dst); },
            [this](std::span<const std::byte> src) { serverToClient.push(src); },
        };
    }
};

/// Runs the handshake on both ends.
struct Connected {
    Link link;
    _<ATlsIdentity> identity = ATlsIdentity::generateSelfSigned("aui-test");
    _<ATlsSession> server;
    _<ATlsSession> client;

    Connected(AOptional<AByteBuffer> expected = std::nullopt) {
        std::exception_ptr serverError;
        std::thread serverThread([&] {
            try {
                server = ATlsSession::accept(link.serverSide(), identity, 5s);
            } catch (...) {
                serverError = std::current_exception();
            }
        });
        ATlsSession::ClientOptions options;
        options.expectedFingerprint = expected ? expected : AOptional<AByteBuffer>(identity->fingerprintSha256());
        try {
            client = ATlsSession::connect(link.clientSide(), options, 5s);
        } catch (...) {
            serverThread.detach();   // unreachable in practice: the tests below build mismatches differently
            throw;
        }
        serverThread.join();
        if (serverError) {
            std::rethrow_exception(serverError);
        }
    }
};

std::string readExactly(ATlsSession& session, std::size_t size) {
    std::string result;
    std::byte buffer[4096];
    while (result.size() < size) {
        const auto n = session.read(std::span(buffer).first(std::min(sizeof(buffer), size - result.size())));
        if (n == 0) {
            break;
        }
        result.append(reinterpret_cast<const char*>(buffer), n);
    }
    return result;
}

}   // namespace

TEST(TlsIdentity, SelfSignedHasStableFingerprint) {
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    EXPECT_EQ(identity->fingerprintSha256().size(), 32u);
    EXPECT_EQ(identity->fingerprintHex().length(), 64u);
    EXPECT_EQ(identity->fingerprintHex(), ATlsIdentity::toHex(identity->fingerprintSha256()));
}

TEST(TlsIdentity, IdentitiesAreDistinct) {
    auto a = ATlsIdentity::generateSelfSigned("aui-test");
    auto b = ATlsIdentity::generateSelfSigned("aui-test");
    EXPECT_NE(a->fingerprintHex(), b->fingerprintHex());
}

TEST(Tls, HandshakeAndEcho) {
    Connected c;
    c.client->write(bytes("ping"));
    EXPECT_EQ(readExactly(*c.server, 4), "ping");
    c.server->write(bytes("pong!"));
    EXPECT_EQ(readExactly(*c.client, 5), "pong!");
}

TEST(Tls, ClientSeesTheServerFingerprint) {
    Connected c;
    ASSERT_TRUE(c.client->peerFingerprintSha256());
    EXPECT_EQ(*c.client->peerFingerprintSha256(), c.identity->fingerprintSha256());
}

TEST(Tls, LargeTransferBothWays) {
    Connected c;
    std::string data(3 * 1024 * 1024, '\0');
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<char>(i * 31 + (i >> 8));
    }
    std::string receivedByServer;
    std::thread reader([&] { receivedByServer = readExactly(*c.server, data.size()); });
    std::thread echo([&] { c.server->write(bytes(data)); });
    c.client->write(bytes(data));
    auto receivedByClient = readExactly(*c.client, data.size());
    reader.join();
    echo.join();
    EXPECT_TRUE(receivedByServer == data);
    EXPECT_TRUE(receivedByClient == data);
}

TEST(Tls, WriteWhileAnotherThreadIsReading) {
    Connected c;
    std::string received;
    std::thread reader([&] { received = readExactly(*c.client, 5); });   // blocks in read
    std::this_thread::sleep_for(100ms);
    c.client->write(bytes("hello"));   // must not wait for the reader
    EXPECT_EQ(readExactly(*c.server, 5), "hello");
    c.server->write(bytes("world"));
    reader.join();
    EXPECT_EQ(received, "world");
}

TEST(Tls, WrongFingerprintIsRejectedByClient) {
    Link link;
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    auto other = ATlsIdentity::generateSelfSigned("aui-test");
    std::thread serverThread([&] {
        try {
            auto s = ATlsSession::accept(link.serverSide(), identity, 5s);
            std::byte c;
            s->read({ &c, 1 });   // returns once the client is gone
        } catch (const AException&) {
        }
    });
    ATlsSession::ClientOptions options;
    options.expectedFingerprint = other->fingerprintSha256();
    EXPECT_THROW((void) ATlsSession::connect(link.clientSide(), options, 5s), ATlsException);
    link.clientToServer.closeWriter();
    serverThread.join();
}

TEST(Tls, CloseNotifyEndsReading) {
    Connected c;
    c.client->write(bytes("bye"));
    c.client->close();
    EXPECT_EQ(readExactly(*c.server, 3), "bye");
    std::byte buffer[8];
    EXPECT_EQ(c.server->read(buffer), 0u);
}

TEST(Tls, TransportEndOfStreamEndsReading) {
    Connected c;
    c.link.clientToServer.closeWriter();
    std::byte buffer[8];
    // either an orderly 0 or an error: the stream is over and must not hang
    try {
        EXPECT_EQ(c.server->read(buffer), 0u);
    } catch (const AException&) {
    }
}

TEST(Tls, CorruptedRecordsAreDetected) {
    Connected c;
    // everything the client sends from now on is damaged
    c.link.clientToServer.corruptAfter(0);
    c.client->write(bytes("tampered"));
    std::byte buffer[32];
    EXPECT_THROW(c.server->read(buffer), ATlsException);
}

TEST(Tls, GarbageInsteadOfClientHelloFails) {
    Link link;
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    const std::string garbage = "GET / HTTP/1.1\r\nHost: x\r\n\r\n";
    link.clientToServer.push(bytes(garbage));
    EXPECT_THROW((void) ATlsSession::accept(link.serverSide(), identity, 2s), ATlsException);
}

TEST(Tls, HandshakeTimesOutWhenThePeerIsSilent) {
    Link link;
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    const auto start = std::chrono::steady_clock::now();
    EXPECT_THROW((void) ATlsSession::accept(link.serverSide(), identity, 300ms), ATlsException);
    EXPECT_LT(std::chrono::steady_clock::now() - start, 3s);
}

TEST(Tls, TransportExceptionsPropagate) {
    Link link;
    auto identity = ATlsIdentity::generateSelfSigned("aui-test");
    auto transport = link.serverSide();
    transport.read = [](std::span<std::byte>) -> std::size_t { throw std::runtime_error("transport is gone"); };
    transport.waitForRead = [](std::chrono::milliseconds) { return true; };
    EXPECT_THROW((void) ATlsSession::accept(transport, identity, 2s), std::runtime_error);
}
