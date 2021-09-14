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
    throw JsonException(formatMiscastException("value is not an object"));
}

AVector<AJsonElement>& JsonValue::asArray()
{
    throw JsonException(formatMiscastException("value is not an array"));
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
