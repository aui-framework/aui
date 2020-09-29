//
// Created by alex2772 on 9/27/20.
//

#include "JsonNull.h"
#include "JsonException.h"

JsonNull::~JsonNull() {

}

bool JsonNull::isVariant() {
    return false;
}

bool JsonNull::isObject() {
    return false;
}

bool JsonNull::isArray() {
    return false;
}

AVariant& JsonNull::asVariant() {
    throw JsonException("null is not a value");
}

AMap<AString, AJsonElement>& JsonNull::asObject() {
    throw JsonException("null is not an object");
}

AVector<AJsonElement>& JsonNull::asArray() {
    throw JsonException("null is not an array");
}

void JsonNull::serialize(const _<IOutputStream>& os) const {
    os->write("null", 4);
}

bool JsonNull::isNull() {
    return true;
}
