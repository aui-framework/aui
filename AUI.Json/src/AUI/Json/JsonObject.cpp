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

void JsonObject::serialize(const _<IOutputStream>& os) const {
    os->write("{", 1);
    bool comma = true;
    for (auto& element : mValue) {
        if (comma) {
            comma = false;
        } else {
            os->write(",", 1);
        }
        os->write("\"", 1);
        {
            auto s = element.first.toStdString();
            os->write(s.c_str(), s.length());
        }
        os->write("\":", 2);
        AJson::write(os, element.second);
    }
    os->write("}", 1);
}
