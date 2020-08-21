#pragma once
#include "AUI/Common/AString.h"
#include "IInputStream.h"

class API_AUI_CORE StringStream: public IInputStream
{
private:
	AString mString;
	AString::iterator mIterator;

	public:
	StringStream(const AString& string);

	int read(char* dst, int size) override;
};
