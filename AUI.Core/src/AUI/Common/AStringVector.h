#pragma once
#include "AString.h"
#include "AVector.h"

class API_AUI_CORE AStringVector: public AVector<AString>
{
public:
	AStringVector()
	{
	}
	AStringVector(const AVector<AString>& v) : AVector<AString>(v)
	{
	}

    template<typename Iterator>
	AStringVector(Iterator begin, Iterator end) : AVector<AString>(begin, end)
	{
	}

	AStringVector(const std::allocator<AString>& allocator)
		: AVector<AString>(allocator)
	{
	}

	AStringVector(p::size_type _Count, const std::allocator<AString>& allocator)
		: AVector<AString>(_Count, allocator)
	{
	}

	AStringVector(p::size_type _Count, const AString& _Val, const std::allocator<AString>& allocator)
		: AVector<AString>(_Count, _Val, allocator)
	{
	}

	AStringVector(std::initializer_list<AString> _Ilist, const std::allocator<AString>& allocator = std::allocator<AString>())
		: AVector<AString>(std::move(_Ilist), allocator)
	{
	}

	AStringVector& noEmptyStrings();

	[[nodiscard]] AString join(wchar_t w) const;

};
