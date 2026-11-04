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
#include "AUI/Curl/Detail/WebsocketFrame.h"

using namespace aui::websocket;

namespace {
AByteBuffer payloadOfSize(std::size_t size) {
    AByteBuffer b;
    b.resize(size);
    for (std::size_t i = 0; i < size; ++i) {
        b.data()[i] = static_cast<char>('a' + i % 26);
    }
    return b;
}

constexpr Mask MASK = {0x12, 0x34, 0x56, 0x78};
}   // namespace

TEST(WebsocketFrameTest, LengthBoundaries) {
    for (std::size_t size : {0u, 1u, 125u, 126u, 127u, 65535u, 65536u, 70000u}) {
        for (bool masked : {false, true}) {
            auto payload = payloadOfSize(size);
            auto encoded = encodeFrame(Opcode::BINARY, payload, true, masked ? AOptional<Mask>(MASK) : std::nullopt);

            FrameDecoder decoder(masked, 1 << 20);
            decoder.feed(encoded);
            auto frame = decoder.next();
            ASSERT_TRUE(frame) << "size=" << size << " masked=" << masked;
            EXPECT_TRUE(frame->fin);
            EXPECT_EQ(frame->opcode, Opcode::BINARY);
            EXPECT_EQ(frame->payload, payload) << "size=" << size << " masked=" << masked;
            EXPECT_FALSE(decoder.next());
        }
    }
}

TEST(WebsocketFrameTest, HeaderSizes) {
    EXPECT_EQ(encodeFrame(Opcode::TEXT, payloadOfSize(125)).size(), 2u + 125);
    EXPECT_EQ(encodeFrame(Opcode::TEXT, payloadOfSize(126)).size(), 4u + 126);
    EXPECT_EQ(encodeFrame(Opcode::TEXT, payloadOfSize(65535)).size(), 4u + 65535);
    EXPECT_EQ(encodeFrame(Opcode::TEXT, payloadOfSize(65536)).size(), 10u + 65536);
    EXPECT_EQ(encodeFrame(Opcode::TEXT, payloadOfSize(5), true, MASK).size(), 2u + 4 + 5);
}

TEST(WebsocketFrameTest, KnownEncoding) {
    // RFC 6455, section 5.7: a single-frame masked text message "Hello"
    auto encoded = encodeFrame(Opcode::TEXT, AByteBuffer::fromString("Hello"), true, Mask { 0x37, 0xfa, 0x21, 0x3d });
    EXPECT_EQ(encoded, AByteBuffer::fromHexString("818537fa213d7f9f4d5158"));
}

TEST(WebsocketFrameTest, ByteByByte) {
    auto payload = payloadOfSize(300);
    auto encoded = encodeFrame(Opcode::TEXT, payload, true, MASK);
    FrameDecoder decoder(true, 1 << 20);
    for (std::size_t i = 0; i < encoded.size(); ++i) {
        EXPECT_FALSE(decoder.next()) << "i=" << i;
        decoder.feed(AByteBufferView(encoded.data() + i, 1));
    }
    auto frame = decoder.next();
    ASSERT_TRUE(frame);
    EXPECT_EQ(frame->payload, payload);
}

TEST(WebsocketFrameTest, SeveralFramesInOneChunk) {
    AByteBuffer chunk;
    chunk << encodeFrame(Opcode::TEXT, AByteBuffer::fromString("one"), true, MASK);
    chunk << encodeFrame(Opcode::PING, AByteBuffer::fromString("two"), true, MASK);
    chunk << encodeFrame(Opcode::BINARY, AByteBuffer::fromString("three"), true, MASK);
    FrameDecoder decoder(true, 1024);
    decoder.feed(chunk);
    EXPECT_EQ(decoder.next()->payload, AByteBuffer::fromString("one"));
    EXPECT_EQ(decoder.next()->opcode, Opcode::PING);
    EXPECT_EQ(decoder.next()->payload, AByteBuffer::fromString("three"));
    EXPECT_FALSE(decoder.next());
}

TEST(WebsocketFrameTest, Fragmentation) {
    FrameDecoder decoder(true, 1024);
    decoder.feed(encodeFrame(Opcode::TEXT, AByteBuffer::fromString("Hel"), false, MASK));
    decoder.feed(encodeFrame(Opcode::CONTINUATION, AByteBuffer::fromString("lo"), true, MASK));
    auto first = decoder.next();
    ASSERT_TRUE(first);
    EXPECT_FALSE(first->fin);
    EXPECT_EQ(first->opcode, Opcode::TEXT);
    auto second = decoder.next();
    ASSERT_TRUE(second);
    EXPECT_TRUE(second->fin);
    EXPECT_EQ(second->opcode, Opcode::CONTINUATION);
}

TEST(WebsocketFrameTest, ServerRejectsUnmaskedFrame) {
    FrameDecoder decoder(true, 1024);
    decoder.feed(encodeFrame(Opcode::TEXT, AByteBuffer::fromString("x")));
    try {
        (void) decoder.next();
        FAIL() << "expected ProtocolError";
    } catch (const ProtocolError& e) {
        EXPECT_EQ(e.closeCode, close_code::PROTOCOL_ERROR);
    }
}

TEST(WebsocketFrameTest, ClientRejectsMaskedFrame) {
    FrameDecoder decoder(false, 1024);
    decoder.feed(encodeFrame(Opcode::TEXT, AByteBuffer::fromString("x"), true, MASK));
    EXPECT_THROW((void) decoder.next(), ProtocolError);
}

TEST(WebsocketFrameTest, ReservedBitsAndUnknownOpcode) {
    {
        FrameDecoder decoder(false, 1024);
        decoder.feed(AByteBuffer::fromHexString("c100"));   // RSV1
        EXPECT_THROW((void) decoder.next(), ProtocolError);
    }
    {
        FrameDecoder decoder(false, 1024);
        decoder.feed(AByteBuffer::fromHexString("8300"));   // opcode 3 is reserved
        EXPECT_THROW((void) decoder.next(), ProtocolError);
    }
}

TEST(WebsocketFrameTest, ControlFrameLimits) {
    {
        FrameDecoder decoder(false, 1 << 20);
        decoder.feed(encodeFrame(Opcode::PING, payloadOfSize(126)));   // > 125
        EXPECT_THROW((void) decoder.next(), ProtocolError);
    }
    {
        FrameDecoder decoder(false, 1 << 20);
        decoder.feed(encodeFrame(Opcode::PING, payloadOfSize(1), false));   // fragmented control frame
        EXPECT_THROW((void) decoder.next(), ProtocolError);
    }
}

TEST(WebsocketFrameTest, OversizedFrameIsRejectedBeforeItArrives) {
    FrameDecoder decoder(true, 1000);
    // only the header of a 70000 byte frame was received
    auto encoded = encodeFrame(Opcode::BINARY, payloadOfSize(70000), true, MASK);
    decoder.feed(AByteBufferView(encoded.data(), 14));
    try {
        (void) decoder.next();
        FAIL() << "expected ProtocolError";
    } catch (const ProtocolError& e) {
        EXPECT_EQ(e.closeCode, close_code::MESSAGE_TOO_BIG);
    }
}

TEST(WebsocketFrameTest, HugeLengthWithMsbSet) {
    FrameDecoder decoder(false, std::numeric_limits<std::size_t>::max());
    decoder.feed(AByteBuffer::fromHexString("827fffffffffffffffff"));
    EXPECT_THROW((void) decoder.next(), ProtocolError);
}
