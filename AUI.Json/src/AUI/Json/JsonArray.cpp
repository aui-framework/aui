#include "JsonArray.h"
#include "JsonException.h"

JsonArray::~JsonArray()
{
}

bool JsonArray::isVariant()
{
	return false;
}

bool JsonArray::isObject()
{
	return true;
}

bool JsonArray::isArray()
{
	return false;
}

AVariant& JsonArray::asVariant()
{
	throw JsonException("array is not a value");
}

AMap<AString, AJsonElement>& JsonArray::asObject()
{
	throw JsonException("array is not an object");
}

AVector<AJsonElement>& JsonArray::asArray()
{
	return mValue;
}

void JsonArray::serialize(const _<IOutputStream>& os) const {
    os->write("[", 1);
    bool comma = true;
    for (auto& element : mValue) {
        if (comma) {
            comma = false;
        } else {
            os->write(",", 1);
        }
        AJson::write(os, element);
    }
    os->write("]", 1);
}
