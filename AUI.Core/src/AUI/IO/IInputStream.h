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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once
#include "EOFException.h"

#include <AUI/Common/AByteBuffer.h>
#include <glm/glm.hpp>

class AByteBuffer;
class IOutputStream;

class API_AUI_CORE IInputStream
{
public:
	virtual ~IInputStream() = default;

	/**
	 * \brief Reads up to <code>size</code> bytes from stream. Implementation must read at least one byte. Blocking
	 *        (waiting for new data) is allowed. Returns 0 if stream has reached the end. Returns -1 if stream has
	 *        encountered an error.
	 * \param dst destination buffer
	 * \param size destination buffer's size. > 0
	 * \return -1 - error
	 *          0 - EOF (end of file)
	 *         >0 - count of read bytes
	 */
	virtual int read(char* dst, int size) = 0;

	/**
	 * \brief Reads up to 0x10000 and stores to AByteBuffer
	 * \param dst destination buffer
	 */
	inline void read(AByteBuffer& dst)
	{
		const size_t BUFFER_SIZE = 0x10000;

		if (dst.getReserved() < BUFFER_SIZE) {
			dst.reserve(BUFFER_SIZE);
		}
		int r = read(dst.getCurrentPosAddress(), BUFFER_SIZE);

		if (r < 0)
			throw IOException();

		if (r == 0)
			throw EOFException();

		dst.setSize(dst.getCurrentPos() + r);
	}

	/**
	 * \brief Reads raw data and stores to out. Does not applicable to types with pointers.
	 * \tparam storage data type
	 * \param out result
	 * \return this
	 */
	template<typename T>
	inline IInputStream& operator>>(T& out)
	{
		auto dst = reinterpret_cast<char*>(&out);

		int accumulator = sizeof(T);
		
		for (int r = 0; accumulator; dst += r, accumulator -= r) {
			r = read(dst, accumulator);
			if (r < 0)
				throw IOException("something went wrong while reading from the stream");
			if (r == 0)
				throw EOFException();
		}
		return *this;
	}

    /**
     * \brief Redirects all this input stream data to output stream.
     * \note This declaration is strange because it overloads the <code>inline IInputStream& operator>>(T& out)</code>
     *       function
     * \tparam base of IOutputStream
     * \param os IOutputStream
     * \return this
     */
    template<typename T>
    inline IInputStream& operator>>(const _<T>& os);


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
            int r = read(buf.data(), length);
            assert(r == length);  // NOLINT(clang-diagnostic-sign-compare)
        }
		return buf;
	}
};

#include "IOutputStream.h"

template<typename T>
inline IInputStream& IInputStream::operator>>(const _<T>& os)
{
    char buf[0x8000];
    for (int r; (r = read(buf, sizeof(buf))) > 0;) {
        os->write(buf, r);
    }
    return *this;
}