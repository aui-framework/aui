// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2772 on 9/5/22.
//

#pragma once


#include "ACurl.h"
#include "AUI/Common/AByteBufferView.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/ADynamicPipe.h"

/**
 * @brief Websocket implementation.
 * @ingroup curl
 */
class API_AUI_CURL AWebsocket: public ACurl, public IOutputStream {
    friend class WebsocketTest_Receive1_Test;
    friend class WebsocketTest_Receive2_Test;
public:
    AWebsocket(const AString& url, AString key = generateKeyString());
    void write(const char* src, size_t size) override;

    void close() override;

    enum class Opcode {
        CONTINUATION = 0x0,
        TEXT = 0x1,
        BINARY = 0x2,
        CLOSE = 0x8,
        PING = 0x9,
        PONG = 0xa,
    };

private:
    AString mKey;
    ADynamicPipe mRead;
    bool mAccepted = false;

    /**
     * @brief Websocket frame header.
     *
     * @details
     * WebSocket is a framed protocol in the format:
     *
     *    0               1                   2                   3
     *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     *   +-+-+-+-+-------+-+-------------+-------------------------------+
     *   |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     *   |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     *   |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     *   | |1|2|3|       |K|             |                               |
     *   +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     *   |     Extended payload length continued, if payload len == 127  |
     *   + - - - - - - - - - - - - - - - +-------------------------------+
     *   |                               |Masking-key, if MASK set to 1  |
     *   +-------------------------------+-------------------------------+
     *   | Masking-key (continued)       |          Payload Data         |
     *   +-------------------------------- - - - - - - - - - - - - - - - +
     *   :                     Payload Data continued ...                :
     *   + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     *   |                     Payload Data continued ...                |
     *   +---------------------------------------------------------------+
     *
     * @see https://tools.ietf.org/html/rfc6455#section-5.2
     */
    struct Header {
        /* first byte: fin + opcode */
        uint8_t opcode: 4;
        uint8_t rsv: 3;
        uint8_t fin: 1;

        /* second byte: mask + payload length */
        uint8_t payload_len: 7; /* if 126, uses extra 2 bytes (uint16_t)
                                 * if 127, uses extra 8 bytes (uint64_t)
                                 * if <=125 is self-contained
                                 */
        uint8_t mask: 1; /* if 1, uses 4 extra bytes */
    };

    static AString generateKeyString();

    void writeRaw(const char* src, size_t size);
    void writeRawMasked(const std::uint8_t mask[4], AByteBufferView message);
    void writeMessage(Opcode opcode, AByteBufferView message);

    std::size_t onDataReceived(AByteBufferView data);
    std::size_t decodeOnePacket(AByteBufferView data);
    std::size_t onDataSend(char* dst, std::size_t maxLen);

    AWebsocket(): ACurl(ACurl::Builder("localhost")) {} // for tests

    AOptional<AByteBuffer> mTrailingBuffer;

signals:
    emits<> connected;
    emits<AByteBuffer> received;
    emits<AString /* message */> websocketClosed;
};

AUI_ENUM_VALUES(AWebsocket::Opcode,
                AWebsocket::Opcode::CONTINUATION,
                AWebsocket::Opcode::TEXT,
                AWebsocket::Opcode::BINARY,
                AWebsocket::Opcode::CLOSE,
                AWebsocket::Opcode::PING,
                AWebsocket::Opcode::PONG
                )
