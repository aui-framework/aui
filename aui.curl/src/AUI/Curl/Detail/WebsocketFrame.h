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

#pragma once

#include <array>
#include <cstdint>
#include <exception>

#include "AUI/ACurl.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/AByteBufferView.h"
#include "AUI/Common/AOptional.h"

namespace aui::websocket {

/**
 * @brief WebSocket frame opcode (RFC 6455, section 5.2).
 */
enum class Opcode : std::uint8_t {
    CONTINUATION = 0x0,
    TEXT = 0x1,
    BINARY = 0x2,
    CLOSE = 0x8,
    PING = 0x9,
    PONG = 0xa,
};

/**
 * @brief Close status codes (RFC 6455, section 7.4.1) used by the codec.
 */
namespace close_code {
constexpr std::uint16_t NORMAL = 1000;
constexpr std::uint16_t PROTOCOL_ERROR = 1002;
constexpr std::uint16_t MESSAGE_TOO_BIG = 1009;
}   // namespace close_code

using Mask = std::array<std::uint8_t, 4>;

struct Frame {
    bool fin = true;
    Opcode opcode = Opcode::TEXT;
    AByteBuffer payload;
};

/**
 * @brief Thrown by FrameDecoder when the peer violates the protocol. `closeCode` is what should be sent in the CLOSE
 * frame before dropping the connection.
 */
class API_AUI_CURL ProtocolError : public std::exception {
public:
    ProtocolError(std::uint16_t closeCode, const char* message) : closeCode(closeCode), mMessage(message) {}
    [[nodiscard]] const char* what() const noexcept override { return mMessage; }

    std::uint16_t closeCode;

private:
    const char* mMessage;
};

/**
 * @brief Serializes a single frame.
 * @param mask when set, the payload is masked and the mask bit is set (clients must do that, servers must not).
 */
API_AUI_CURL AByteBuffer encodeFrame(Opcode opcode, AByteBufferView payload, bool fin = true, AOptional<Mask> mask = std::nullopt);

/**
 * @brief Incremental frame parser.
 * @details
 * Feed it with whatever the socket gave, then pull frames with next() until it returns an empty optional. Header
 * fields are read byte by byte, so the result does not depend on the host endianness or struct layout.
 *
 * next() throws ProtocolError for: reserved bits set, unknown opcodes, fragmented or oversized control frames,
 * a mask bit that does not match `requireMasked`, and frames bigger than `maxPayloadSize`.
 */
class API_AUI_CURL FrameDecoder {
public:
    /**
     * @param requireMasked true on the server side (client frames must be masked), false on the client side.
     * @param maxPayloadSize upper bound for a single frame payload.
     */
    FrameDecoder(bool requireMasked, std::size_t maxPayloadSize) : mRequireMasked(requireMasked), mMaxPayloadSize(maxPayloadSize) {}

    void feed(AByteBufferView data);

    /**
     * @return the next complete frame (already unmasked) or nothing if more bytes are needed.
     */
    [[nodiscard]] AOptional<Frame> next();

private:
    bool mRequireMasked;
    std::size_t mMaxPayloadSize;
    AByteBuffer mBuffer;
    std::size_t mOffset = 0;
};

}   // namespace aui::websocket
