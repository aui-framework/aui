// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#pragma once

#include <AUI/Traits/serializable.h>

/**
 * @brief Acts like std::string_view but for AByteBuffer.
 * @ingroup core
 * @note don't use const reference of AByteBufferView. Passing by const reference forces compiler to use memory instead
 * of registers.
 * @note AByteBufferView is intended for const access to memory data. As a function argument, consider to use
 * `std::span<std::byte>` instead for non-const access.
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
    AByteBufferView slice(std::size_t offset) const noexcept {
        return slice(offset, size() - offset);
    }

    [[nodiscard]]
    AByteBufferView slice(std::size_t offset, std::size_t size) const noexcept {
        assert(("out of bounds", offset + size <= mSize));
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


    _<IInputStream> toStream() const;
};

template<>
struct ASerializable<AByteBufferView> {
    static void write(IOutputStream& os, AByteBufferView view) {
        os.write(view.data(), view.size());
    }
};