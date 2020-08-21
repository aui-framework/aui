#pragma once
#include <stdexcept>

class AXmlParseError: public std::runtime_error
{
public:
	AXmlParseError(const std::string& _Message)
		: runtime_error(_Message)
	{
	}

	AXmlParseError(const char* _Message)
		: runtime_error(_Message)
	{
	}
};
