#pragma once
#include "IJsonElement.h"

class JsonObject : public IJsonElement
{
private:
	AMap<AString, AJsonElement> mValue;

public:
	explicit JsonObject(const AMap<AString, AJsonElement>& value)
		: mValue(value)
	{
	}
	JsonObject()
	{
	}

	~JsonObject() override;
	bool isVariant() override;
	bool isObject() override;
	bool isArray() override;
	AVariant& asVariant() override;
	AMap<AString, AJsonElement>& asObject() override;
	AVector<AJsonElement>& asArray() override;
};
