#pragma once
#include "IJsonElement.h"

class JsonValue: public IJsonElement
{
private:
	AVariant mValue;
	
public:
	explicit JsonValue(const AVariant& value)
		: mValue(value)
	{
	}

	JsonValue()
	{
	}

	~JsonValue() override;
	bool isVariant() override;
	bool isObject() override;
	bool isArray() override;
	AVariant& asVariant() override;
	AMap<AString, AJsonElement>& asObject() override;
	AVector<AJsonElement>& asArray() override;
};
