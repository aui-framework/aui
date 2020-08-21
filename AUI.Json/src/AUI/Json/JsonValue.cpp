#include "JsonValue.h"
#include "JsonException.h"

JsonValue::~JsonValue()
{
}

bool JsonValue::isVariant()
{
	return true;
}

bool JsonValue::isObject()
{
	return false;
}

bool JsonValue::isArray()
{
	return false;
}

AVariant& JsonValue::asVariant()
{
	return mValue;
}

AMap<AString, AJsonElement>& JsonValue::asObject()
{
	throw JsonException("value is not an object");
}

AVector<AJsonElement>& JsonValue::asArray()
{
	throw JsonException("value is not an array");
}
