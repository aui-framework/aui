#pragma once
#include "IOException.h"

class FileNotFoundException: public IOException
{
public:
	FileNotFoundException(char const* _Message)
		: IOException(_Message)
	{
	}
};
