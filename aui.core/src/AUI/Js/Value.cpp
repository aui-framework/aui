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

#include "Value.h"

#include <cstdint>
#include <cstring>

namespace aui::js {

AStringView typeName(const Value& value) noexcept {
    return std::visit(
        [](const auto& v) -> AStringView {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, Undefined>) return "undefined";
            if constexpr (std::is_same_v<T, bool>) return "boolean";
            if constexpr (std::is_same_v<T, double>) return "number";
            if constexpr (std::is_same_v<T, AString>) return "string";
            if constexpr (std::is_same_v<T, AByteBuffer>) return "bytes";
            if constexpr (std::is_same_v<T, Array>) return "array";
            if constexpr (std::is_same_v<T, Object>) return "object";
        },
        static_cast<const Value::variant&>(value));
}

namespace {

class Reader {
public:
    explicit Reader(std::span<const std::byte> data) : mData(data) {}

    template <typename T>
    T read() {
        T result;
        std::memcpy(&result, take(sizeof(T)).data(), sizeof(T));
        return result;
    }

    AString readString() {
        auto bytes = take(read<std::uint32_t>());
        return AString(std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size()));
    }

    Value readValue() {
        using wire::Tag;
        switch (Tag(read<std::uint8_t>())) {
            case Tag::UNDEFINED: return Undefined {};
            case Tag::FALSE: return false;
            case Tag::TRUE: return true;
            case Tag::NUMBER: return read<double>();
            case Tag::STRING: return readString();
            case Tag::BYTES: {
                auto* address = reinterpret_cast<std::byte*>(std::uintptr_t(read<std::uint64_t>()));
                const auto length = std::size_t(read<std::uint64_t>());
                return AByteBuffer::adopt({ address, address ? length : 0 });
            }
            case Tag::ARRAY: {
                Array array;
                const auto count = read<std::uint32_t>();
                array.reserve(count);
                for (std::uint32_t i = 0; i < count; ++i) array.push_back(readValue());
                return array;
            }
            case Tag::OBJECT: {
                Object object;
                const auto count = read<std::uint32_t>();
                for (std::uint32_t i = 0; i < count; ++i) {
                    auto key = readString();
                    object[std::move(key)] = readValue();
                }
                return object;
            }
        }
        throw AException("aui::js: malformed value, unknown tag");
    }

    [[nodiscard]]
    bool atEnd() const noexcept {
        return mData.empty();
    }

private:
    std::span<const std::byte> mData;

    std::span<const std::byte> take(std::size_t size) {
        if (size > mData.size()) {
            throw AException("aui::js: malformed value, {} bytes past the end"_format(size - mData.size()));
        }
        auto result = mData.first(size);
        mData = mData.subspan(size);
        return result;
    }
};

}   // namespace

Value wire::decode(std::span<const std::byte> block) {
    Reader reader(block);
    auto value = reader.readValue();
    if (!reader.atEnd()) {
        throw AException("aui::js: malformed value, trailing bytes");
    }
    return value;
}

}   // namespace aui::js
