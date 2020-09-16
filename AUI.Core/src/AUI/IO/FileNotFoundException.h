#pragma once
#include "IOException.h"

class FileNotFoundException: public IOException
{
public:
	FileNotFoundException(const AString& message);
};
