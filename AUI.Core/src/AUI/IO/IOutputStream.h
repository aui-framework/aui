#pragma once

#include <cstring>
#include "EOFException.h"
#include "AUI/Common/AString.h"

class IInputStream;

class API_AUI_CORE IOutputStream
{
public:
	virtual ~IOutputStream() = default;
	virtual int write(const char* src, int size) = 0;

	inline void write(const _<AByteBuffer>& buffer)
	{
		write(buffer->getCurrentPosAddress(), buffer->getAvailable());
	}


	template<typename T>
	inline IOutputStream& operator<<(const T& out)
	{
		if (write(reinterpret_cast<const char*>(&out), sizeof(T)) < 0)
			throw IOException("could not write to file");
		return *this;
	}

    template<typename T>
    inline IOutputStream& operator<<(const _<T>& is);
	
	inline IOutputStream& operator<<(const AString& out)
	{
		if (out.empty())
			return *this;
		auto st = out.toStdString();
		write(st.c_str(), st.length());
		return *this;
	}
	inline IOutputStream& operator<<(const char* out)
	{
		if (!out)
			return *this;
		write(out, strlen(out));
		return *this;
	}


	inline void writeSizedBuffer(const _<AByteBuffer>& buffer)
	{
		*this << uint32_t(buffer->getSize());
		write(buffer->data(), buffer->getSize());
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