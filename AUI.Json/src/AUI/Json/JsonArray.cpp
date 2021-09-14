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
	return false;
}

bool JsonArray::isArray()
{
	return true;
}

AVariant& JsonArray::asVariant()
{
    throw JsonException(formatMiscastException("array is not a value"));
}

AMap<AString, AJsonElement>& JsonArray::asObject()
{
    throw JsonException(formatMiscastException("array is not an object"));
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
