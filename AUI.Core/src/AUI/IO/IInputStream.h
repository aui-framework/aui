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