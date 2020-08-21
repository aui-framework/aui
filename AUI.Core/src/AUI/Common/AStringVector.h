#pragma once
#include "AString.h"
#include "AVector.h"
#include "AUI/Model/IListModel.h"

class API_AUI_CORE AStringVector: public AVector<AString>, public IListModel
{
public:
	AStringVector()
	{
	}
	AStringVector(const AVector<AString>& v) : AVector<AString>(v)
	{
	}

	AStringVector(const std::allocator<AString>& allocator)
		: AVector<AString>(allocator)
	{
	}

	AStringVector(parent::size_type _Count, const std::allocator<AString>& allocator)
		: AVector<AString>(_Count, allocator)
	{
	}

	AStringVector(parent::size_type _Count, const AString& _Val, const std::allocator<AString>& allocator)
		: AVector<AString>(_Count, _Val, allocator)
	{
	}

	AStringVector(std::initializer_list<AString> _Ilist, const std::allocator<AString>& allocator = std::allocator<AString>())
		: AVector<AString>(std::move(_Ilist), allocator)
	{
	}

	AStringVector& noEmptyStrings();

	[[nodiscard]] AString join(wchar_t w) const;


	~AStringVector() override;
	
	size_t listSize() override;
	AVariant listItemAt(size_t index) override;
};
