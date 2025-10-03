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

#include <curl/curl.h>
#include "AWebsocket.h"
#include "AUI/Common/AByteBufferView.h"
#include "AUI/Common/AString.h"
#include "AUI/Util/ARandom.h"
#include "AUI/Crypt/AHash.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/IO/AByteBufferInputStream.h"

/*
 * Thanks to https://github.com/barbieri/barbieri-playground/blob/master/curl-websocket/curl-websocket.c
 */

using namespace std::chrono_literals;

namespace {
    static std::default_random_engine gRandomEngine;

    static inline void myHton(uint8_t *mem, uint8_t len)
    {
        uint8_t *bytes;
        uint8_t i, mid;

        if (len % 2) return;

        mid = len / 2;
        bytes = mem;
        for (i = 0; i < mid; i++) {
            uint8_t tmp = bytes[i];
            bytes[i] = bytes[len - i - 1];
            bytes[len - i - 1] = tmp;
        }
    }
}

AWebsocket::AWebsocket(const AString& url, AString key):
ACurl(ACurl::Builder(url.replacedAll("wss://", "https://").replacedAll("ws://", "http://"))
    .withHeaders({
        "Expect: 101",
        "Transfer-Encoding:",
        "Connection: Upgrade",
        "Upgrade: websocket",
        "Sec-WebSocket-Version: 13",
        "Sec-WebSocket-Key: {}"_format(AByteBuffer::fromString(key).toBase64String()),
        "Sec-WebSocket-Protocol: chat"
        })
    .withHttpVersion(ACurl::Http::VERSION_1_1)
    .withUpload(true)
    .withLowSpeedLimit(1)
    .withCustomRequest("GET")
    .withHeaderCallback([this](AByteBufferView v) {
        auto asStr = AString(v.toStdStringView());
        if (asStr.startsWith("Sec-WebSocket-Accept: ")) {
            asStr = asStr.trimRight('\n').trimRight('\r');
            auto serverKeyBase64 = asStr.substr(asStr.find(": ") + 2);
            auto serverKeySha1 = AByteBuffer::fromBase64String(serverKeyBase64);
            auto clientKeySha1 = AHash::sha1(AByteBuffer::fromString(AByteBuffer::fromString(mKey).toBase64String() + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
            if (serverKeySha1 != clientKeySha1) {
                throw AException("websocket key mismatch");
            }
            mAccepted = true;
        } else if (asStr == "\r\n") {
            if (!mAccepted) {
                throw AException("server didn't accept websocket connection");
            }
            emit connected;
        }
    }).withWriteCallback([this] (AByteBufferView v) {
        return onDataReceived(v);
    }).withBody([this](char* dst, std::size_t maxLen) {
        return onDataSend(dst, maxLen);
    })), mKey(std::move(key)) {

}


std::size_t AWebsocket::onDataReceived(AByteBufferView data) {
    auto actuallyRead = [this](AByteBufferView data) -> size_t {
        auto it = data.begin();
        while (it != data.end()) {
            auto o = decodeOnePacket(AByteBufferView::fromRange(it, data.end()));
            if (o == 0) {
                break;
            }
            it += o;
        }
        return std::distance(data.begin(), it);
    };

    if (mTrailingBuffer) {
        *mTrailingBuffer << data;
        auto r = actuallyRead(*mTrailingBuffer);
        if (r == mTrailingBuffer->size()) {
            mTrailingBuffer.reset();
        } else {
            mTrailingBuffer->erase(mTrailingBuffer->begin(), mTrailingBuffer->begin() + r);
        }
    } else {
        auto r = actuallyRead(data);
        if (r != data.size()) {
            mTrailingBuffer.emplace(data.slice(r));
        }
    }

    // (https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html) Your callback should return the number of bytes actually
    // taken care of. If that amount differs from the amount passed to your callback function, it signals an error
    // condition to the [libcurl] library. This causes the transfer to get aborted and the libcurl function used returns
    // CURLE_WRITE_ERROR.
    return data.size();
}



std::size_t AWebsocket::onDataSend(char* dst, std::size_t maxLen) {
    if (mRead.empty()) {
        return 0;
    }
    return mRead.read(dst, maxLen);
}


AString AWebsocket::generateKeyString() {
    AString s;
    s.resize(16);
    for (auto& v : s.bytes()) {
        v = std::uniform_int_distribution(int('a'), int('z'))(gRandomEngine);
    }
    return s;
}

void AWebsocket::writeRaw(const char* src, size_t size) {
    mRead.write(src, size);
    curl_easy_pause(handle(), 0);
}

void AWebsocket::writeMessage(AWebsocket::Opcode opcode, AByteBufferView message) {
    Header h;
    h.fin = 1;
    h.rsv = 0;
    h.opcode = static_cast<uint8_t>(opcode);
    h.mask = 1;
    h.payload_len = ((message.size() > std::numeric_limits<std::uint16_t>::max())
                     ? 127
                     : (message.size() > 125) ? 126 : message.size());

    writeRaw(reinterpret_cast<const char*>(&h), sizeof(h));

    if (h.payload_len == 127) {
        std::uint64_t payload_len = message.size();
        myHton(reinterpret_cast<uint8_t*>(&payload_len), sizeof(payload_len));
        writeRaw(reinterpret_cast<const char*>(&payload_len), sizeof(payload_len));
    } else if (h.payload_len == 126) {
        std::uint16_t payload_len = message.size();
        myHton(reinterpret_cast<uint8_t*>(&payload_len), sizeof(payload_len));
        writeRaw(reinterpret_cast<const char*>(&payload_len), sizeof(payload_len));
    }

    std::uint8_t mask[4];
    for (auto& v : mask) v = std::uniform_int_distribution(int(std::numeric_limits<std::uint8_t>::min()),
                                                           int(std::numeric_limits<std::uint8_t>::max()))(gRandomEngine);
    writeRaw(reinterpret_cast<const char*>(mask), sizeof(mask));

    writeRawMasked(mask, message);
}

void AWebsocket::writeRawMasked(const std::uint8_t* mask, AByteBufferView message) {
    AByteBuffer temporaryBuffer;
    temporaryBuffer.resize(message.size());

    for (std::size_t i = 0; i < message.size(); ++i) {
        temporaryBuffer.at<std::uint8_t>(i) = message.at<std::uint8_t>(i) ^ mask[i % 4];
    }

    writeRaw(temporaryBuffer.data(), temporaryBuffer.size());
}

void AWebsocket::write(const char* src, size_t size) {
    writeMessage(Opcode::TEXT, { src, size });
}

void AWebsocket::close() {
    writeMessage(Opcode::CLOSE, {nullptr, 0});
    ACurl::close();
}

std::size_t AWebsocket::decodeOnePacket(AByteBufferView data) {
    Header h;
    if (data.size() < sizeof(Header)) {
        return 0; // not enough
    }

    const char* it = data.begin();
    h = *reinterpret_cast<const Header*>(it);

    if (h.mask) { // 5.1 "client MUST close a connection if it detects a masked frame"
        ALogger::err("websocket") << "Received masked frame, closing connection";
        close();
        return data.size();
    }

    it += sizeof(Header);

    uint16_t payloadLength;

    switch (h.payload_len) {
        case 126:
        case 127: {
            if (it + sizeof(payloadLength) > data.end()) {
                return 0; // not enough
            }
            payloadLength = *reinterpret_cast<const std::uint64_t*>(it);
            myHton(reinterpret_cast<uint8_t*>(&payloadLength), sizeof(payloadLength));
            it += sizeof(payloadLength);
            break;
        }
        default:
            payloadLength = h.payload_len;
    }

    switch (h.opcode) {
        case int(Opcode::BINARY):
        case int(Opcode::TEXT):
        case int(Opcode::CONTINUATION):
            break;

        case int(Opcode::CLOSE): {
            emit websocketClosed(std::string_view(it + 2, h.payload_len - 2));
            close();
            return 0;
        }

        case int(Opcode::PING):
            writeMessage(Opcode::PONG, {});
            break;

        case int(Opcode::PONG):
            break;

        default:
            ALogger::err("AWebsocket") << "Unknown opcode: " << AString::numberHex(h.opcode) << ", closing connection";
            close();
            return 0;
    }

    if (std::distance(it, data.end()) < payloadLength) {
        // not enough
        return 0;
    }

    emit received(AByteBuffer(it, payloadLength));

    it += payloadLength;

    return std::distance(data.begin(), it);
}

