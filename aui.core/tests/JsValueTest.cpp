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
#include <AUI/Js/Value.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace aui::js;
using wire::Tag;

namespace {

/// Writes the wire format as `Module.auiSettle` does in the browser.
class Writer {
public:
    Writer& tag(Tag t) { return raw(std::uint8_t(t)); }
    Writer& number(double v) { return tag(Tag::NUMBER).raw(v); }
    Writer& string(std::string_view s) { return tag(Tag::STRING).utf8(s); }
    Writer& array(std::uint32_t count) { return tag(Tag::ARRAY).raw(count); }
    Writer& object(std::uint32_t count) { return tag(Tag::OBJECT).raw(count); }
    Writer& key(std::string_view s) { return utf8(s); }

    /// Like JavaScript: the bytes go into a block of their own from malloc, the value refers to it.
    Writer& bytes(std::string_view s) {
        void* block = s.empty() ? nullptr : std::malloc(s.size());
        if (block) std::memcpy(block, s.data(), s.size());
        return tag(Tag::BYTES).raw(std::uint64_t(reinterpret_cast<std::uintptr_t>(block))).raw(std::uint64_t(s.size()));
    }

    [[nodiscard]] std::span<const std::byte> block() const { return mData; }

private:
    std::vector<std::byte> mData;

    template <typename T>
    Writer& raw(T v) {
        const auto* p = reinterpret_cast<const std::byte*>(&v);
        mData.insert(mData.end(), p, p + sizeof(T));
        return *this;
    }

    Writer& utf8(std::string_view s) {
        raw(std::uint32_t(s.size()));
        const auto* p = reinterpret_cast<const std::byte*>(s.data());
        mData.insert(mData.end(), p, p + s.size());
        return *this;
    }
};

AString text(const AByteBuffer& b) { return AString(std::string_view(b.data(), b.size())); }

}   // namespace

TEST(JsValue, Scalars) {
    EXPECT_TRUE(std::holds_alternative<Undefined>(wire::decode(Writer().tag(Tag::UNDEFINED).block())));
    EXPECT_EQ(from<bool>(wire::decode(Writer().tag(Tag::TRUE).block())), true);
    EXPECT_EQ(from<bool>(wire::decode(Writer().tag(Tag::FALSE).block())), false);
    EXPECT_EQ(from<double>(wire::decode(Writer().number(0.25).block())), 0.25);
    EXPECT_EQ(from<int>(wire::decode(Writer().number(-42).block())), -42);
    EXPECT_EQ(from<AString>(wire::decode(Writer().string("Привет, мир").block())), "Привет, мир");
}

TEST(JsValue, BytesAreAdoptedNotCopied) {
    Writer w;
    w.bytes("pixels");
    std::uint64_t address;
    std::memcpy(&address, w.block().data() + 1, sizeof(address));

    auto buffer = from<AByteBuffer>(wire::decode(w.block()));
    EXPECT_EQ(text(buffer), "pixels");
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(buffer.data()), address);   // the very block JavaScript allocated
    EXPECT_TRUE(from<AByteBuffer>(wire::decode(Writer().bytes("").block())).empty());
}

TEST(JsValue, TupleOfAnArray) {
    // [js_promise_tuple]
    Writer w;
    w.array(3).number(640).number(480).bytes("rgba");   // JavaScript: [640, 480, new Uint8Array(...)]
    auto [width, height, pixels] = from<std::tuple<std::uint32_t, std::uint32_t, AByteBuffer>>(wire::decode(w.block()));
    // [js_promise_tuple]
    EXPECT_EQ(width, 640);
    EXPECT_EQ(height, 480);
    EXPECT_EQ(text(pixels), "rgba");
}

TEST(JsValue, Containers) {
    auto object = from<AMap<AString, AVector<int>>>(
        wire::decode(Writer().object(2).key("a").array(2).number(1).number(2).key("b").array(0).block()));
    EXPECT_EQ(object["a"], (AVector<int> { 1, 2 }));
    EXPECT_TRUE(object["b"].empty());

    auto optional = from<AVector<AOptional<AString>>>(
        wire::decode(Writer().array(2).tag(Tag::UNDEFINED).string("x").block()));
    EXPECT_EQ(optional, (AVector<AOptional<AString>> { std::nullopt, AString("x") }));
}

TEST(JsValue, MismatchSaysWhere) {
    auto mismatch = [](std::span<const std::byte> block, auto type) -> AString {
        try {
            (void)from<decltype(type)>(wire::decode(block));
        } catch (const TypeMismatchException& e) {
            return e.getMessage();
        }
        return "no exception";
    };
    EXPECT_EQ(mismatch(Writer().string("x").block(), AByteBuffer {}), "expected bytes, got string");
    EXPECT_EQ(mismatch(Writer().array(2).number(1).string("x").block(), std::tuple<int, int> {}),
              "[1]: expected number, got string");
    EXPECT_EQ(mismatch(Writer().array(1).object(1).key("pixels").string("x").block(),
                       AVector<AMap<AString, AByteBuffer>> {}),
              "[0].pixels: expected bytes, got string");
    EXPECT_EQ(mismatch(Writer().array(1).number(1).block(), std::tuple<int, int> {}),
              "expected an array of 2, got 1");
    EXPECT_EQ(mismatch(Writer().number(0.5).block(), int {}), "expected an integer that fits 4 bytes, got 0.5");
}

TEST(JsValue, MalformedBlockThrows) {
    EXPECT_THROW(wire::decode(Writer().array(2).number(1).block()), AException);   // one item missing
    EXPECT_THROW(wire::decode(Writer().number(1).number(2).block()), AException);  // trailing bytes
}

TEST(AByteBuffer, AdoptsMallocAndGrowsIt) {
    auto* block = static_cast<std::byte*>(std::malloc(3));
    std::memcpy(block, "abc", 3);
    auto buffer = AByteBuffer::adopt({ block, 3 });
    buffer.write("defgh", 5);   // realloc keeps the adopted contents
    EXPECT_EQ(text(buffer), "abcdefgh");
}
