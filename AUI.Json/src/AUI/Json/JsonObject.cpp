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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

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
    throw JsonException(formatMiscastException("object is not a value"));
}

AMap<AString, AJsonElement>& JsonObject::asObject()
{
	return mValue;
}

AVector<AJsonElement>& JsonObject::asArray()
{
    throw JsonException(formatMiscastException("object is not an array"));
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
