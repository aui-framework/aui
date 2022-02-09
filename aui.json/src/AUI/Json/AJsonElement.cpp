/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include <AUI/IO/IOutputStream.h>
#include "AJsonElement.h"
#include "JsonValue.h"
#include "JsonObject.h"
#include "JsonArray.h"
#include "JsonNull.h"

AJsonElement::AJsonElement(const _<IJsonElement>& json_element): mJson(json_element)
{
}

AJsonElement::AJsonElement(std::nullptr_t) :
        mJson(_new<JsonNull>()) {

}

bool AJsonElement::isNull() const {
    return !mJson || mJson->isNull();
}


bool AJsonElement::isVariant() const
{
	return mJson->isVariant();
}

bool AJsonElement::isObject() const
{
	return mJson->isObject();
}

bool AJsonElement::isArray() const
{
	return mJson->isArray();
}

bool AJsonElement::isString() const {
    return mJson->isVariant() && mJson->asVariant().isString();
}
bool AJsonElement::isBool() const {
    return mJson->isVariant() && mJson->asVariant().isBool();
}
bool AJsonElement::isInt() const {
    return mJson->isVariant() && mJson->asVariant().isInt();
}


const AVariant& AJsonElement::asVariant() const
{
	return mJson->asVariant();
}

int AJsonElement::asInt() const
{
	return mJson->asVariant().toInt();
}

int AJsonElement::asBool() const {
    return mJson->asVariant().toBool();
}

AString AJsonElement::asString() const
{
	return mJson->asVariant().toString();
}

const AMap<AString, AJsonElement>& AJsonElement::asObject() const
{
    return mJson->asObject();
}

const AVector<AJsonElement>& AJsonElement::asArray() const
{
	return mJson->asArray();
}

const AJsonElement& AJsonElement::operator[](size_t index) const
{
	return mJson->asArray()[index];
}

AJsonElement AJsonElement::operator[](const AString& key) const
{
    if (auto c = mJson->asObject().contains(key)) {
        return c->second;
    }
    return AJsonElement{_new<JsonNull>()};
}

void AJsonElement::serialize(IOutputStream& param) const {
    mJson->serialize(param);
}


AJsonElement& AJsonElement::operator=(const AVariant& value) {
    mJson = _new<JsonValue>(value);
    return *this;
}

AJsonElement::AJsonElement():
    mJson(_new<JsonNull>())
{

}


AJsonValue::AJsonValue(const AVariant& value)
{
    if (value.isNull()) {
        mJson = _new<JsonNull>();
    } else {
        mJson = _new<JsonValue>(value);
    }
}

AJsonObject::AJsonObject(const AMap<AString, AJsonElement>& value):
        AJsonElement(_new<JsonObject>(value))
{
}

AJsonObject::AJsonObject():
    AJsonElement(_new<JsonObject>())
{

}

AJsonElement& AJsonObject::operator[](const AString& key) {
    return mJson->asObject()[key];
}

AJsonArray::AJsonArray(const AVector<AJsonElement>& value) :
	AJsonElement(_new<JsonArray>(value))
{
}


AJsonArray& AJsonArray::operator<<(const AJsonElement& value) {
    mJson->asArray().push_back(value);
    return *this;
}

void AJsonArray::push_back(const AJsonElement& value) {
    mJson->asArray().push_back(value);
}

AJsonArray::AJsonArray():
    AJsonElement(_new<JsonArray>())
{

}

AJsonArray AJsonArray::fromVariantArray(const AVector<AVariant>& value) {
    AVector<AJsonElement> a;
    a.reserve(value.size());
    for (auto& v : value) {
        a << AJsonValue(v);
    }
    return AJsonArray(a);
}

bool AJsonArray::empty() const {
    return mJson->asArray().empty();
}



