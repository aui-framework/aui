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
