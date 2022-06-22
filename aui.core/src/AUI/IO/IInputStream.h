/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once
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
     * @brief Reads up to <code>size</code> bytes from stream. Implementation must read at least one byte. Blocking
     *        (waiting for new data) is allowed.
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
