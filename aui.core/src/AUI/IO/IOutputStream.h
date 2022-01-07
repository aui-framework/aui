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
#include "AEOFException.h"
#include "AUI/Common/AString.h"

class IInputStream;

class API_AUI_CORE IOutputStream
{
public:
	virtual ~IOutputStream() = default;

    /**
     * \brief Writes exact <code>size</code> bytes to stream. Blocking (waiting for write all data) is allowed. Returns
     *        -1 if stream has encountered an error.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd>An implementation can throw any exception that subclasses <a href="#AIOException">AIOException</a>.</dd>
     * </dl>
     * \param dst source buffer
     * \param size source buffer's size. > 0
     * \return count of written bytes
     */
	virtual size_t write(const char* src, size_t size) = 0;

	/**
     * \brief Redirects all input stream data to this output stream
     * \param os IInputStream
     * \param limit max count of bytes to write
     * \note if you don't want to limit stream, use the operator<<() function.
	 */
	 template<typename T>
	inline void writeAll(const _<T>& fis, size_t limit = (std::numeric_limits<size_t>::max)()) {
        writeAll(*fis, limit);
    }

    /**
     * \brief Redirects all input stream data to this output stream
     * \param os IInputStream
     * \param limit max count of bytes to write
     * \note if you don't want to limit stream, use the operator<<() function.
     */
    inline void writeAll(IInputStream& fis, size_t limit = (std::numeric_limits<size_t>::max)());

    /**
     * \brief Redirects all input stream data to this output stream
     * \param os IInputStream
     * \param limit max count of bytes to write
     * \note if you don't want to limit stream, use the operator<<() function.
     */
    inline void writeAll(IInputStream&& fis, size_t limit = (std::numeric_limits<size_t>::max)()) {
        writeAll(fis, limit);
    }


	/**
	 * \brief Works as <code>write(const char* src, int size)</code> except it accepts AByteBuffer instead of
	 *        const char* and int.
	 * \param buffer source data buffer
	 */
	inline void write(const AByteBuffer& buffer)
	{
		write(buffer.getCurrentPosAddress(), buffer.getAvailable());
	}


    /**
     * \brief Writes raw data from in. Does not applicable to types with pointers.
     * \tparam storage data type
     * \param in data. Can be <a href="#AByteBuffer">AByteBuffer</a>
     * \return this
     */
	template<typename T>
	inline IOutputStream& operator<<(const T& in)
	{
        if constexpr (std::is_same_v<AByteBuffer, T>) {
            if (write(in.data(), in.getSize()) < 0)
                throw AIOException("could not write to file");
        } else if constexpr (std::is_base_of_v<IInputStream, T>) {
            writeAll(const_cast<T&>(in));
        } else {
            static_assert(std::is_standard_layout_v<T>, "data is too complex to be written to stream");
            if (write(reinterpret_cast<const char *>(&in), sizeof(T)) != sizeof(T))
                throw AIOException("could not write to file");
        }
		return *this;
	}

    /**
     * \brief Redirects all input stream data to this output stream.
     * \note This declaration is strange because it overloads the
     *       <code>inline IOutputStream& operator<<(const T& in)</code> function
     * \tparam base of IOutputStream
     * \param os IOutputStream
     * \return this
     */
    template<typename T>
    inline IOutputStream& operator<<(const _<T>& is) {
        operator<<(*is);
        return *this;
    }

    /**
     * \brief Redirects all input stream data to this output stream.
     * \note This declaration is strange because it overloads the
     *       <code>inline IOutputStream& operator<<(const T& in)</code> function
     * \tparam base of IOutputStream
     * \param os IOutputStream
     * \return this
     */
    inline IOutputStream& operator<<(IInputStream& is);


    /**
     * \brief Writes the AByteBuffer in the following format: uint32_t as size, %size% bytes...
     * \param buffer AByteBuffer to be written
     * \see IInputStream::readSizedBuffer
     */
	inline void writeSizedBuffer(const AByteBuffer& buffer)
	{
		*this << uint32_t(buffer.getSize());
		write(buffer.data(), buffer.getSize());
	}


	/**
	 * \brief Encodes string to utf-8 and writes it as it should be written; not garbage
	 * \param out string to be written
	 * \return this
	 */
    inline IOutputStream& operator<<(const AString& out)
    {
        if (out.empty())
            return *this;
        auto st = out.toStdString();
        write(st.c_str(), st.length());
        return *this;
    }
    /**
     * \brief Writes string as it should be written; not garbage
     * \param out string to be written
     * \return this
     */
    inline IOutputStream& operator<<(const char* out)
    {
        if (!out)
            return *this;
        write(out, strlen(out));
        return *this;
    }
};


#include "IInputStream.h"

inline IOutputStream& IOutputStream::operator<<(IInputStream& is)
{
    char buf[0x8000];
    for (size_t r; (r = is.read(buf, sizeof(buf))) > 0;) {
        write(buf, r);
    }
    return *this;
}


void IOutputStream::writeAll(IInputStream& fis, size_t limit) {
    char buf[0x800];
    while (limit > 0) {
        size_t r = fis.read(buf, (glm::min)(sizeof(buf), limit));
        if (r == 0) {
            return;
        }
        write(buf, r);

        if (r >= limit) {
            return;
        }
        limit -= r;
    }
}
