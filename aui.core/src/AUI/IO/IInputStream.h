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
#include "AEOFException.h"

#include <AUI/Common/AByteBuffer.h>
#include <glm/glm.hpp>

class IOutputStream;

class API_AUI_CORE IInputStream
{
public:
    virtual ~IInputStream() = default;

    /**
     * @brief Reads up to <code>size</code> bytes from stream. Implementation must read at least one byte. Blocking
     *        (waiting for new data) is allowed. Returns 0 if stream has reached the end. Returns -1 if stream has
     *        encountered an error.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd>An implementation can throw any exception that subclasses <a href="#AIOException">AIOException</a>.</dd>
     * </dl>
     * @param dst destination buffer
     * @param size destination buffer's size. > 0
     * @return  0 - EOF (end of file)
     *         >0 - count of read bytes
     */
    virtual size_t read(char* dst, size_t size) = 0;

    /**
     * \brief Reads up to 0x10000 and puts them AByteBuffer
     * \param dst destination buffer
     */
    inline void readBuffer(AByteBuffer& dst, size_t bufferSize = 0x10000)
    {
        if (dst.getReserved() - dst.getCurrentPos() < size_t(bufferSize)) {
            dst.reserve(dst.getCurrentPos() + bufferSize);
        }
        size_t r = read(dst.getCurrentPosAddress(), bufferSize);

        if (r == 0)
            throw AEOFException();

        dst.setSize(dst.getCurrentPos() + r);
        dst.setCurrentPos(dst.getCurrentPos() + bufferSize);
    }

    /**
     * \brief Reads raw data and stores to out. Does not applicable to types with pointers.
     * \tparam storage data type
     * \param out result
     * \return this
     */
    template<typename T>
    inline IInputStream& operator>>(T& out);

    /**
     * \brief Redirects all this input stream data to output stream.
     * \note This declaration is strange because it overloads the <code>inline IInputStream& operator>>(T& out)</code>
     *       function
     * \tparam base of IOutputStream
     * \param os IOutputStream
     * \return this
     */
    template<typename T>
    inline IInputStream& operator>>(const _<T>& os) {
        operator>>(*os);
        return *this;
    }

    /**
     * \brief Reads the AByteBuffer in the following format: uint32_t as size, %size% bytes...
     * \see IOutputStream::writeSizedBuffer
     * \return produced AByteBuffer
     */
    inline AByteBuffer readSizedBuffer() {
        AByteBuffer buf;
        uint32_t length;
        *this >> length;
        if (length) {
            buf.reserve(length);
            buf.setSize(length);
            size_t r = read(buf.data(), length);
            assert(r == length);  // NOLINT(clang-diagnostic-sign-compare)
        }
        return buf;
    }
};


#include "IOutputStream.h"

template<typename T>
IInputStream& IInputStream::operator>>(T& out) {
    if constexpr (std::is_base_of_v<IOutputStream, T>) {
        out << *this;
    } else {
        static_assert(std::is_standard_layout_v<T>, "data is too complex to be read from stream");
        auto dst = reinterpret_cast<char*>(&out);

        size_t accumulator = sizeof(T);

        for (size_t r = 0; accumulator; dst += r, accumulator -= r) {
            r = read(dst, accumulator);
            if (r < 0)
                throw AIOException("something went wrong while reading from the stream");
            if (r == 0)
                throw AEOFException();
        }
    }
    return *this;
}
