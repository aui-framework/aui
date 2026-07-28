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

#include <AUI/Traits/serializable.h>

/**
 * @brief Acts like std::string_view but for AByteBuffer.
 * @ingroup core
 * @details
 *
 * !!! note
 *
 *     don't use const reference of AByteBufferView. Passing by const reference forces compiler to use memory instead
 *     of registers.
 *
 *
 * !!! note
 *
 *     AByteBufferView is intended for const access to memory data. As a function argument, consider to use
 *     `std::span<std::byte>` instead for non-const access.
 */
class API_AUI_CORE AByteBufferView {
private:
    const char* mBuffer;
    size_t mSize;

public:
    AByteBufferView() noexcept: mBuffer(nullptr), mSize(0) {}
    AByteBufferView(const char* buffer, size_t size) noexcept: mBuffer(buffer), mSize(size) {}
    explicit AByteBufferView(const std::string& string) noexcept: mBuffer(string.data()), mSize(string.size()) {}
    explicit AByteBufferView(std::string_view string) noexcept: mBuffer(string.data()), mSize(string.size()) {}

    static AByteBufferView fromRange(const char* begin, const char* end) noexcept {
        return AByteBufferView(begin, std::distance(begin, end));
    }

    /**
     * @brief Gets value of specified type by byte index relative to the beginning of internal buffer.
     * @tparam T data type
     * @param byteIndex byte offset realtive to the beginning of internal buffer
     * @return data
     */
    template <typename T>
    const T& at(size_t byteIndex) const noexcept
    {
        return *reinterpret_cast<const T*>(mBuffer + byteIndex);
    }

    [[nodiscard]]
    bool empty() const noexcept {
        return mSize == 0;
    }

    [[nodiscard]]
    AByteBufferView slice(std::size_t offset /* to end */) const {
        return slice(offset, size() - offset);
    }

    [[nodiscard]]
    AByteBufferView slice(std::size_t offset, std::size_t size) const {
        AUI_ASSERTX(offset + size <= mSize, "out of bounds");
        return { mBuffer + offset, size };
    }

    [[nodiscard]]
    const char* data() const noexcept {
        return mBuffer;
    }

    [[nodiscard]]
    size_t size() const noexcept {
        return mSize;
    }

    [[nodiscard]]
    auto begin() const noexcept {
        return data();
    }
    [[nodiscard]]
    auto end() const noexcept {
        return data() + size();
    }

    [[nodiscard]]
    AString toHexString() const;

    [[nodiscard]]
    AString toBase64String() const;

    template<typename T>
    [[nodiscard]]
    const T& as() const {
        if (mSize != sizeof(T)) {
            throw AException("as<T>(): invalid size");
        }
        return *reinterpret_cast<const T*>(mBuffer);
    }

    template<typename T>
    [[nodiscard]]
    static AByteBufferView fromRaw(const T& data) noexcept {
        return { reinterpret_cast<const char*>(&data), sizeof(data) };
    }

    [[nodiscard]]
    std::string_view toStdStringView() const noexcept {
        return { data(), size() };
    }


    _unique<IInputStream> toStream() const;
};

inline std::ostream& operator<<(std::ostream& lhs, const AByteBufferView& rhs) {
    lhs << "[";
    for (const auto b : rhs) {
        char buf[8];
        #if defined(FMT_VERSION) && (FMT_VERSION < 100000)
        lhs.write(buf, std::distance(std::begin(buf), fmt::format_to(buf, " {:02x}", b)));
        #else
        auto end = fmt::format_to(buf, " {:02x}", b);
        lhs.write(buf, end - buf);
        #endif
    }
    lhs << " ]";

    return lhs;
}

template<>
struct ASerializable<AByteBufferView> {
    static void write(IOutputStream& os, AByteBufferView view) {
        os.write(view.data(), view.size());
    }
};

template<>
struct std::hash<AByteBufferView>
{
    size_t operator()(AByteBufferView t) const noexcept
    {
        if (t.data() == nullptr || t.size() == 0) {
            return 0;
        }
        constexpr size_t FNV_offset_basis = 14695981039346656037ULL;
        constexpr size_t FNV_prime = 1099511628211ULL;

        size_t hash = FNV_offset_basis;
        for (size_t i = 0; i < t.size(); ++i) {
            hash ^= static_cast<size_t>(t.data()[i]);
            hash *= FNV_prime;
        }
        return hash;
    }
};
