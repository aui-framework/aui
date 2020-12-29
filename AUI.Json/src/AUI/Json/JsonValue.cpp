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

void JsonValue::serialize(const _<IOutputStream>& os) const {
    auto myStr = mValue.toString();
    if (mValue.getType() == AVariantType::AV_STRING) {
        os->write("\"", 1);
        myStr = myStr.replacedAll("\"", "\\\"");
        auto s = myStr.toStdString();
        os->write(s.c_str(), s.length());
        os->write("\"", 1);
    } else {
        auto s = myStr.toStdString();
        os->write(s.c_str(), s.length());
    }
}
