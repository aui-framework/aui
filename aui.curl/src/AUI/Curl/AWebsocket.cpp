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

#include "AWebsocket.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <random>
#include <span>

#include "AUI/Crypt/AHash.h"
#include "AUI/Crypt/ATlsSession.h"
#include "AUI/IO/AIOException.h"
#include "AUI/Network/AInet4Address.h"
#include "AUI/Network/ATcpSocket.h"

using namespace aui::websocket;
using namespace std::chrono_literals;

namespace {
constexpr auto GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
constexpr auto POLL_INTERVAL = 100ms;

/// Scheme, host, port and "path?query" of a ws:// or wss:// url.
struct Target {
    bool secure;
    AString host;
    std::uint16_t port;
    AString resource;
};

Target parse(const AString& url) {
    const auto schemeEnd = url.find("://");
    const AString scheme = schemeEnd == AString::NPOS ? AString {} : AString(url.substr(0, schemeEnd));
    if (scheme != "ws" && scheme != "wss") {
        throw AException("not a websocket url: " + url);
    }
    Target result { .secure = scheme == "wss", .resource = "/" };
    auto authority = url.substr(schemeEnd + 3);
    if (auto slash = authority.find('/'); slash != AString::NPOS) {
        result.resource = authority.substr(slash);
        authority = authority.substr(0, slash);
    } else if (auto query = authority.find('?'); query != AString::NPOS) {
        result.resource = "/" + authority.substr(query);
        authority = authority.substr(0, query);
    }
    result.port = result.secure ? 443 : 80;
    if (auto colon = authority.find(':'); colon != AString::NPOS) {
        result.port = authority.substr(colon + 1).toUInt().valueOr(result.port);
        authority = authority.substr(0, colon);
    }
    result.host = std::move(authority);
    return result;
}

template <std::size_t N>
std::array<std::uint8_t, N> randomBytes() {
    std::random_device device;
    std::array<std::uint8_t, N> result;
    for (auto& v : result) {
        v = std::uint8_t(device());
    }
    return result;
}
}   // namespace

AWebsocket::AWebsocket(AString url) : AWebsocket(std::move(url), Options {}) {}

AWebsocket::AWebsocket(AString url, Options options)
  : mUrl(std::move(url)), mOptions(std::move(options)), mDecoder(false, mOptions.maxMessageSize) {}

AWebsocket::AWebsocket() : mDecoder(false, mOptions.maxMessageSize) {}

AWebsocket::~AWebsocket() { close(); }

void AWebsocket::connect() {
    auto target = parse(mUrl);
    mSocket = _new<ATcpSocket>(AInet4Address::fromString(target.host, target.port));
    mSocket->setTimeout(mOptions.connectTimeout);
    if (target.secure) {
        ATlsTransport transport {
            [socket = mSocket](std::chrono::milliseconds t) { return socket->waitForRead(t); },
            [socket = mSocket](std::span<std::byte> dst) { return socket->read(dst); },
            [socket = mSocket](std::span<const std::byte> src) {
                socket->write(reinterpret_cast<const char*>(src.data()), src.size());
            },
        };
        ATlsSession::ClientOptions options;
        options.expectedFingerprint = mOptions.expectedFingerprint;
        options.serverName = target.host;
        mTls = ATlsSession::connect(std::move(transport), options, mOptions.connectTimeout);
    }
    handshake(target.host, target.resource);
    emit connected;
}

void AWebsocket::handshake(const AString& host, const AString& resource) {
    const auto nonce = randomBytes<16>();
    const auto key = AByteBuffer(reinterpret_cast<const char*>(nonce.data()), nonce.size()).toBase64String();
    const auto request = "GET {} HTTP/1.1\r\nHost: {}\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                         "Sec-WebSocket-Version: 13\r\nSec-WebSocket-Key: {}\r\n\r\n"_format(resource, host, key)
                             .toStdString();
    if (mTls) {
        mTls->write(std::as_bytes(std::span(request)));
    } else {
        mSocket->write(request.data(), request.size());
    }

    // byte by byte: what follows the head are frames
    std::string head;
    char c;
    while (head.find("\r\n\r\n") == std::string::npos && readSome(&c, 1) == 1) {
        head += c;
    }
    if (!head.starts_with("HTTP/1.1 101")) {
        throw AWebsocketHandshakeException(AString::fromUtf8(head.substr(0, head.find("\r\n"))));
    }

    const auto expected = AHash::sha1(AByteBuffer::fromString(key + GUID)).toBase64String().toStdString();
    auto lower = head;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](char ch) { return char(std::tolower(ch)); });
    const auto header = lower.find("sec-websocket-accept:");
    if (header == std::string::npos || head.find(expected, header) == std::string::npos) {
        throw AWebsocketHandshakeException("websocket key mismatch");
    }
}

void AWebsocket::run() {
    connect();
    try {
        while (!mCloseRequested && !mPeerGone) {
            pull(POLL_INTERVAL);
            while (auto message = nextMessage()) {
                emit received(std::move(*message));
            }
        }
    } catch (const AIOException&) {
        if (!mCloseRequested) {
            throw;
        }
    }
}

AOptional<AByteBuffer> AWebsocket::read(std::chrono::milliseconds timeout) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    for (;;) {
        if (auto message = nextMessage()) {
            return message;
        }
        if (mPeerGone || mCloseRequested) {
            throw AIOException("the connection is closed");
        }
        const auto left = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());
        if (left <= 0ms) {
            return std::nullopt;
        }
        pull(left);
    }
}

std::size_t AWebsocket::readSome(char* dst, std::size_t size) {
    return mTls ? mTls->read(std::as_writable_bytes(std::span(dst, size))) : mSocket->read(dst, size);
}

void AWebsocket::pull(std::chrono::milliseconds timeout) {
    if (!mSocket->waitForRead(timeout)) {
        return;
    }
    std::array<char, 1 << 15> buffer;   // at least a TLS record: what is left of it is invisible to poll()
    const auto n = readSome(buffer.data(), buffer.size());
    if (n == 0) {
        mPeerGone = true;
        return;
    }
    mDecoder.feed(AByteBufferView(buffer.data(), n));
}

AOptional<AByteBuffer> AWebsocket::nextMessage() {
    while (auto frame = mDecoder.next()) {
        switch (frame->opcode) {
            case Opcode::TEXT:
            case Opcode::BINARY:
                if (mAssemblingActive) {
                    throw ProtocolError(close_code::PROTOCOL_ERROR, "new message inside a fragmented one");
                }
                if (frame->fin) {
                    return std::move(frame->payload);
                }
                mAssembling = std::move(frame->payload);
                mAssemblingActive = true;
                break;
            case Opcode::CONTINUATION:
                if (!mAssemblingActive) {
                    throw ProtocolError(close_code::PROTOCOL_ERROR, "unexpected continuation frame");
                }
                if (mAssembling.size() + frame->payload.size() > mOptions.maxMessageSize) {
                    throw ProtocolError(close_code::MESSAGE_TOO_BIG, "message is too big");
                }
                mAssembling.write(frame->payload.data(), frame->payload.size());
                if (frame->fin) {
                    mAssemblingActive = false;
                    return std::exchange(mAssembling, {});
                }
                break;
            case Opcode::PING:
                // RFC 6455 5.5.3: a Pong carries the application data of the Ping
                send(Opcode::PONG, frame->payload);
                break;
            case Opcode::PONG:
                break;
            case Opcode::CLOSE: {
                mPeerGone = true;
                const auto reason = frame->payload.size() > 2 ? frame->payload.slice(2) : AByteBufferView {};
                emit websocketClosed(AString::fromUtf8(reason));
                close();
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}

std::size_t AWebsocket::onDataReceived(AByteBufferView data) {
    mDecoder.feed(data);
    while (auto message = nextMessage()) {
        emit received(std::move(*message));
    }
    return data.size();
}

void AWebsocket::send(Opcode opcode, AByteBufferView payload) {
    const auto frame = encodeFrame(opcode, payload, true, randomBytes<4>());
    std::unique_lock lock(mSendMutex);
    if (!mSocket) {
        throw AIOException("websocket is not connected");
    }
    if (mTls) {
        mTls->write(std::as_bytes(std::span(frame.data(), frame.size())));
    } else {
        mSocket->write(frame.data(), frame.size());
    }
}

void AWebsocket::writeText(AStringView text) { send(Opcode::TEXT, { text.data(), text.size() }); }

void AWebsocket::writeBinary(AByteBufferView data) { send(Opcode::BINARY, data); }

void AWebsocket::close() {
    if (mCloseRequested.exchange(true) || !mSocket) {
        return;
    }
    try {
        send(Opcode::CLOSE, {});
    } catch (...) {
        // the connection is already broken
    }
    if (mTls) {
        mTls->close();
    }
    mSocket->close();
}
