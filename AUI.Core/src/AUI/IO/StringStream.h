#pragma once
#include "AUI/Common/AString.h"
#include "IInputStream.h"

class API_AUI_CORE StringStream: public IInputStream
{
private:
	std::string mString;
	std::string::iterator mIterator;

	public:
	StringStream(const AString& string);

	int read(char* dst, int size) override;
};
