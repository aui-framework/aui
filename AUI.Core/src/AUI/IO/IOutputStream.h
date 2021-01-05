#pragma once

#include <cstring>
#include "EOFException.h"
#include "AUI/Common/AString.h"

class IInputStream;

class API_AUI_CORE IOutputStream
{
public:
	virtual ~IOutputStream() = default;

    /**
     * \brief Writes exact <code>size</code> bytes to stream. Blocking (waiting for write all data) is allowed. Returns
     *        -1 if stream has encountered an error.
     * \param dst source buffer
     * \param size source buffer's size. > 0
     * \return -1 - error
     *         >0 - count of written bytes
     */
	virtual int write(const char* src, int size) = 0;

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
     * \param in data
     * \return this
     */
	template<typename T>
	inline IOutputStream& operator<<(const T& in)
	{
		if (write(reinterpret_cast<const char*>(&in), sizeof(T)) < 0)
			throw IOException("could not write to file");
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
    inline IOutputStream& operator<<(const _<T>& is);


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
};


#include "IInputStream.h"

template<typename T>
inline IOutputStream& IOutputStream::operator<<(const _<T>& is)
{
    char buf[0x8000];
    for (int r; (r = is->read(buf, sizeof(buf))) > 0;) {
        write(buf, r);
    }
    return *this;
}