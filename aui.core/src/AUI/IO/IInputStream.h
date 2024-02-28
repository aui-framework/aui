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

#pragma once

#include <span>
#include "AEOFException.h"
#include <AUI/Traits/values.h>
#include <glm/glm.hpp>

class IOutputStream;

/**
 * @brief Represents an input stream.
 * @ingroup io
 */
class IInputStream: public aui::noncopyable
{
public:
    virtual ~IInputStream() = default;

    /**
     * @brief Reads up to <code>size</code> bytes from stream. Blocking (waiting for new data) is allowed.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd>An implementation can throw any exception that subclasses <a href="#AIOException">AIOException</a>.</dd>
     * </dl>
     * @param dst destination buffer
     * @param size destination buffer's size. > 0
     * @return number of read bytes (including 0)
     */
    virtual size_t read(char* dst, size_t size) = 0;

    /**
     * @brief Reads up to <code>destination.size()</code> bytes from stream. Blocking (waiting for new data) is allowed.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd>An implementation can throw any exception that subclasses <a href="#AIOException">AIOException</a>.</dd>
     * </dl>
     * @param dst destination buffer
     * @param size destination buffer's size. > 0
     * @return number of read bytes (including 0)
     */
    size_t read(std::span<std::byte> destination) {
        return read((char*)destination.data(), destination.size());
    }

    /**
     * @brief Reads exact <code>size</code> bytes from stream. Blocking (waiting for new data) is allowed.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd>An implementation can throw any exception that subclasses <a href="#AIOException">AIOException</a>.</dd>
     * </dl>
     * @throws Throws <a href="#AEOFException">AEOFException</a> when could not read EXACT <code>size</code> bytes.
     * @param dst destination buffer.
     * @param size destination buffer's size.
     */
    void readExact(char* dst, size_t size) {
        char* begin = dst;
        char* end = dst + size;
        while (begin != end) {
            size_t r = read(begin, end - begin);
            if (r == 0) {
                throw AEOFException();
            }
            begin += r;
        }
    }

    /**
     * Reads data using AUI serialization (see AUI/Traits/serializable.h)
     * @param t value to write
     */
    template<typename T>
    T deserialize();

    /**
     * Reads data using AUI serialization (see AUI/Traits/serializable.h)
     * @param t value to write
     */
    template<typename T>
    IInputStream& operator>>(T& dst);
    /**
     * Reads data using AUI serialization (see AUI/Traits/serializable.h)
     * @param t value to write
     */
    template<typename T>
    IInputStream& operator>>(T&& dst);
};

#include <AUI/Traits/serializable.h>

template<typename T>
inline T IInputStream::deserialize() {
    return aui::deserialize<T>(*this);
}

template<typename T>
IInputStream& IInputStream::operator>>(T& dst) {
    aui::deserialize<T>(*this, dst);
    return *this;
}

template<typename T>
IInputStream& IInputStream::operator>>(T&& dst) {
    aui::deserialize<T>(*this, dst);
    return *this;
}
