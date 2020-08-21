#pragma once
#include "IOException.h"

class EOFException: public IOException
{
public:
	EOFException() : IOException("the stream has reached eof") {}

	virtual ~EOFException() = default;
};
