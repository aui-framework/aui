#include "AStringVector.h"


AStringVector& AStringVector::noEmptyStrings()
{
	remove("");
	return *this;
}

AString AStringVector::join(wchar_t w) const
{
	AString res;

	for (const auto& x : *this)
	{
		if (!res.empty())
		{
			res += w;
		}
		res += x;
	}
	
	return res;
}

AStringVector::~AStringVector()
{
}

size_t AStringVector::listSize()
{
	return size();
}

AVariant AStringVector::listItemAt(size_t index)
{
	return (*this)[index];
}
