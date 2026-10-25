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

#include "WebsocketFrame.h"

#include <cstring>

namespace aui::websocket {

namespace {
bool isControl(Opcode opcode) { return static_cast<std::uint8_t>(opcode) >= 0x8; }

bool isKnown(std::uint8_t opcode) {
    switch (static_cast<Opcode>(opcode)) {
        case Opcode::CONTINUATION:
        case Opcode::TEXT:
        case Opcode::BINARY:
        case Opcode::CLOSE:
        case Opcode::PING:
        case Opcode::PONG:
            return true;
    }
    return false;
}
}   // namespace

AByteBuffer encodeFrame(Opcode opcode, AByteBufferView payload, bool fin, AOptional<Mask> mask) {
    AByteBuffer out;
    out.reserve(payload.size() + 14);

    auto put = [&](std::uint8_t b) { out.write(reinterpret_cast<const char*>(&b), 1); };

    put(static_cast<std::uint8_t>((fin ? 0x80 : 0x00) | static_cast<std::uint8_t>(opcode)));

    const std::uint8_t maskBit = mask ? 0x80 : 0x00;
    if (payload.size() <= 125) {
        put(maskBit | static_cast<std::uint8_t>(payload.size()));
    } else if (payload.size() <= 0xffff) {
        put(maskBit | 126);
        put(static_cast<std::uint8_t>(payload.size() >> 8));
        put(static_cast<std::uint8_t>(payload.size()));
    } else {
        put(maskBit | 127);
        const std::uint64_t size = payload.size();
        for (int shift = 56; shift >= 0; shift -= 8) {
            put(static_cast<std::uint8_t>(size >> shift));
        }
    }

    if (mask) {
        for (auto b : *mask) {
            put(b);
        }
        const auto offset = out.size();
        out.resize(offset + payload.size());
        for (std::size_t i = 0; i < payload.size(); ++i) {
            out.data()[offset + i] = static_cast<char>(static_cast<std::uint8_t>(payload.data()[i]) ^ (*mask)[i % 4]);
        }
    } else if (!payload.empty()) {
        out.write(payload.data(), payload.size());
    }
    return out;
}

void FrameDecoder::feed(AByteBufferView data) {
    if (mOffset > 0) {
        mBuffer.erase(mBuffer.begin(), mBuffer.begin() + mOffset);
        mOffset = 0;
    }
    if (!data.empty()) {
        mBuffer.write(data.data(), data.size());
    }
}

AOptional<Frame> FrameDecoder::next() {
    const auto available = mBuffer.size() - mOffset;
    if (available < 2) {
        return std::nullopt;
    }
    const auto* p = reinterpret_cast<const std::uint8_t*>(mBuffer.data()) + mOffset;

    const bool fin = p[0] & 0x80;
    if (p[0] & 0x70) {
        throw ProtocolError(close_code::PROTOCOL_ERROR, "reserved bits are set");
    }
    const auto opcodeRaw = static_cast<std::uint8_t>(p[0] & 0x0f);
    if (!isKnown(opcodeRaw)) {
        throw ProtocolError(close_code::PROTOCOL_ERROR, "unknown opcode");
    }
    const auto opcode = static_cast<Opcode>(opcodeRaw);

    const bool masked = p[1] & 0x80;
    if (masked != mRequireMasked) {
        throw ProtocolError(close_code::PROTOCOL_ERROR, masked ? "unexpected masked frame" : "frame must be masked");
    }

    std::size_t headerSize = 2;
    std::uint64_t payloadSize = p[1] & 0x7f;
    if (payloadSize == 126) {
        headerSize += 2;
        if (available < headerSize) {
            return std::nullopt;
        }
        payloadSize = (std::uint64_t(p[2]) << 8) | p[3];
    } else if (payloadSize == 127) {
        headerSize += 8;
        if (available < headerSize) {
            return std::nullopt;
        }
        payloadSize = 0;
        for (int i = 0; i < 8; ++i) {
            payloadSize = (payloadSize << 8) | p[2 + i];
        }
        if (payloadSize >> 63) {
            throw ProtocolError(close_code::PROTOCOL_ERROR, "payload length has the most significant bit set");
        }
    }

    if (isControl(opcode) && (!fin || payloadSize > 125)) {
        throw ProtocolError(close_code::PROTOCOL_ERROR, "invalid control frame");
    }
    if (payloadSize > mMaxPayloadSize) {
        throw ProtocolError(close_code::MESSAGE_TOO_BIG, "frame is too big");
    }

    Mask mask {};
    if (masked) {
        if (available < headerSize + 4) {
            return std::nullopt;
        }
        std::memcpy(mask.data(), p + headerSize, 4);
        headerSize += 4;
    }

    if (available - headerSize < payloadSize) {
        return std::nullopt;
    }

    Frame frame;
    frame.fin = fin;
    frame.opcode = opcode;
    frame.payload.resize(static_cast<std::size_t>(payloadSize));
    for (std::size_t i = 0; i < payloadSize; ++i) {
        frame.payload.data()[i] = static_cast<char>(p[headerSize + i] ^ (masked ? mask[i % 4] : 0));
    }
    mOffset += headerSize + static_cast<std::size_t>(payloadSize);
    return frame;
}

}   // namespace aui::websocket
