#pragma once

#include <cstring>
#include "EOFException.h"
#include "AUI/Common/AString.h"

class API_AUI_CORE IOutputStream
{
public:
	virtual ~IOutputStream() = default;
	virtual int write(const char* dst, int size) = 0;

	inline void write(const _<ByteBuffer>& buffer)
	{
		write(buffer->getCurrentPosAddress(), buffer->getAvailable());
	}


	template<typename T>
	IOutputStream& operator<<(const T& out)
	{
		if (write(reinterpret_cast<const char*>(&out), sizeof(T)) < 0)
			throw IOException("could not write to file");
		return *this;
	}
	
	IOutputStream& operator<<(const AString& out)
	{
		if (out.empty())
			return *this;
		write(out.toStdString().c_str(), out.length());
		return *this;
	}
	IOutputStream& operator<<(const char* out)
	{
		if (!out)
			return *this;
		write(out, strlen(out));
		return *this;
	}


	inline void writeSizedBuffer(const _<ByteBuffer>& buffer)
	{
		*this << uint32_t(buffer->getSize());
		write(buffer->data(), buffer->getSize());
	}
};
