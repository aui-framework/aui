#include <curl/curl.h>
#include "AWebsocket.h"
#include "AUI/Util/ARandom.h"
#include "AUI/Crypt/AHash.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/IO/AByteBufferInputStream.h"

/*
 * Thanks to https://github.com/barbieri/barbieri-playground/blob/master/curl-websocket/curl-websocket.c
 */

namespace {
    static std::default_random_engine gRandomEngine;

    static inline void myHton(uint8_t *mem, uint8_t len)
    {
#if __BYTE_ORDER__ != __BIG_ENDIAN
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
#endif
    }
}

AWebsocket::AWebsocket(const AString& url, AString key):
ACurl(ACurl::Builder(url.replacedAll("wss://", "https://"))
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
    .withCustomRequest("GET")
    .withHeaderCallback([this](AByteBufferView v) {
        auto asStr = AString::fromUtf8(v);
        ALogger::debug("curl") << asStr;
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
    }).withReadCallback([this](char* dst, std::size_t maxLen) {
        return onDataSend(dst, maxLen);
    })), mKey(std::move(key)) {

}


std::size_t AWebsocket::onDataReceived(AByteBufferView data) {
    auto begin = data.begin();
    auto end = data.end();

    Header h;
    if (!mLastHeader) {
        if (data.size() < sizeof(Header)) {
            return 0; // not enough
        }
        h = *reinterpret_cast<const Header*>(begin);
        begin += sizeof(Header);
        if (h.payload_len > data.size()) {
            mLastHeader = h;
        }
        switch (h.opcode) {
            case int(Opcode::BINARY):
            case int(Opcode::TEXT):
            case int(Opcode::CONTINUATION):
                break;

            case int(Opcode::CLOSE):
            case int(Opcode::PING):
            case int(Opcode::PONG):
                ALogger::err("websocket") << "Unsupported opcode: " << (Opcode)h.opcode;
                break;

            default:
                ALogger::err("websocket") << "Unknown opcode: " << AString::numberHex(h.opcode);
                break;
        }
    }

    std::size_t dataToRead = glm::min(std::size_t(std::distance(begin, end)),
                                      h.payload_len - mLastPayload.size());
    mLastPayload << AByteBufferView(begin, dataToRead);
    begin += dataToRead;
    assert(begin <= end);

    assert(mLastPayload.size() <= h.payload_len);

    if (mLastPayload.size() == h.payload_len) {
        emit received(AByteBufferView(mLastPayload));
        mLastPayload.clear();
        mLastHeader.reset();
    }

    return std::distance(data.begin(), begin);
}



std::size_t AWebsocket::onDataSend(char* dst, std::size_t maxLen) {
    if (mRead.available() == 0) {
        return 0;
    }
    return mRead.read(dst, maxLen);
}


AString AWebsocket::generateKeyString() {
    AString s;
    s.resize(16);
    for (auto& v : s) {
        v = std::uniform_int_distribution('a', 'z')(gRandomEngine);
    }
    return s;
}

void AWebsocket::writeRaw(const char* src, size_t size) {
    curl_easy_pause(handle(), 0);
    mRead.write(src, size);
    curl_easy_pause(handle(), 0);
}

void AWebsocket::writeMessage(AWebsocket::Opcode opcode, AByteBufferView message) {
    Header h;
    h.fin = 1;
    h.opcode = static_cast<uint8_t>(opcode);
    h.mask = 1;
    h.payload_len = ((message.size() > std::numeric_limits<std::uint16_t>::max())
                     ? 127
                     : (message.size() > 125) ? 126 : message.size());

    std::uint8_t mask[4];
    for (auto& v : mask) v = std::uniform_int_distribution(std::numeric_limits<std::uint8_t>::min(), std::numeric_limits<std::uint8_t>::max())(gRandomEngine);
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

    writeRaw(reinterpret_cast<const char*>(mask), sizeof(mask));

    return writeRawMasked(mask, message);
}

void AWebsocket::writeRawMasked(const std::uint8_t* mask, AByteBufferView message) {
    const uint8_t* itrBegin = reinterpret_cast<const uint8_t*>(message.data());
    const uint8_t* itr = itrBegin;
    const uint8_t* itrEnd = itr + message.size();
    uint8_t tmpbuf[4096];

    while (itr < itrEnd) {
        uint8_t *o = tmpbuf, *o_end = tmpbuf + sizeof(tmpbuf);
        for (; o < o_end && itr < itrEnd; o++, itr++) {
            *o = *itr ^ mask[(itr - itrBegin) & 0x3];
        }
        writeRaw(reinterpret_cast<const char*>(tmpbuf), o - tmpbuf);
    }
}

void AWebsocket::write(const char* src, size_t size) {
    writeMessage(Opcode::TEXT, { src, size });
}
