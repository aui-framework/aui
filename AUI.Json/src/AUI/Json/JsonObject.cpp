#include "JsonObject.h"
#include "JsonException.h"

JsonObject::~JsonObject()
{
}

bool JsonObject::isVariant()
{
	return false;
}

bool JsonObject::isObject()
{
	return true;
}

bool JsonObject::isArray()
{
	return false;
}

AVariant& JsonObject::asVariant()
{
	throw JsonException("object is not a value");
}

AMap<AString, AJsonElement>& JsonObject::asObject()
{
	return mValue;
}

AVector<AJsonElement>& JsonObject::asArray()
{
	throw JsonException("object is not an array");
}
