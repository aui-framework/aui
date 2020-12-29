#pragma once
#include "IJsonElement.h"

class JsonArray : public IJsonElement
{
private:
	AVector<AJsonElement> mValue;

public:
	explicit JsonArray(const AVector<AJsonElement>& value)
		: mValue(value)
	{
	}
	JsonArray()
	{
	}

	~JsonArray() override;
	bool isVariant() override;
	bool isObject() override;
	bool isArray() override;
	AVariant& asVariant() override;
	AMap<AString, AJsonElement>& asObject() override;
	AVector<AJsonElement>& asArray() override;

    void serialize(const _<IOutputStream>& os) const override;
};
