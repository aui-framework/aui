/**
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

#include <cstring>
#include <AUI/Traits/values.h>
#include "AEOFException.h"


class IOutputStream: public aui::noncopyable
{
public:
	virtual ~IOutputStream() = default;

    /**
     * \brief Writes exact <code>size</code> bytes to stream. Blocking (waiting for write all data) is allowed.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd>An implementation can throw any exception that subclasses <a href="#AIOException">AIOException</a>.</dd>
     * </dl>
     * \param dst source buffer
     * \param size source buffer's size. > 0
     */
	virtual void write(const char* src, size_t size) = 0;

    /**
     * Writes data using AUI serialization (see AUI/Traits/serializable.h)
     * @param t value to write
     */
    template<typename T>
    void write(const T& t);

    /**
     * Writes data using AUI serialization (see AUI/Traits/serializable.h)
     * @param t value to write
     */
    template<typename T>
    IOutputStream& operator<<(const T& t) {
        write<T>(t);
        return *this;
    }
};

#include <AUI/Traits/serializable.h>

template<typename T>
void IOutputStream::write(const T& t) {
    aui::serialize(*this, t);
}