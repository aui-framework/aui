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

#pragma once

#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Common/AVariant.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/AVector.h"
#include <AUI/IO/IOutputStream.h>
#include <AUI/Thread/AThreadPool.h>
#include <AUI/Model/IListModel.h>
#include <AUI/Model/AListModel.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/Data/AModelMeta.h>

class IJsonElement;
class AJsonElement;

namespace AJson
{
    void API_AUI_JSON write(_<IOutputStream> os, const AJsonElement& json);
}


class API_AUI_JSON AJsonElement {

    API_AUI_JSON friend void AJson::write(_<IOutputStream> os, const AJsonElement& json);

protected:
    void serialize(_<IOutputStream> param) const;

    _<IJsonElement> mJson;
public:
	explicit AJsonElement(const _<IJsonElement>& json_element);
	AJsonElement(std::nullptr_t);
	AJsonElement();

	AJsonElement& operator=(const AVariant& value);
	AJsonElement& operator=(const AJsonElement& value) = default;

	[[nodiscard]] bool isVariant() const;
	[[nodiscard]] bool isObject() const;
	[[nodiscard]] bool isArray() const;
	[[nodiscard]] bool isNull() const;
	[[nodiscard]] bool isString() const;
	[[nodiscard]] bool isBool() const;
	[[nodiscard]] bool isInt() const;

	[[nodiscard]] bool contains(const AString& key) const {
	    return asObject().contains(key);
	}

	[[nodiscard]] const AVariant& asVariant() const;
	[[nodiscard]] int asInt() const;
	[[nodiscard]] int asBool() const;
	[[nodiscard]] AString asString() const;
	[[nodiscard]] AString asStringOrDefault(const AString& d = "") const {
	    try {
	        return asString();
	    } catch (...) {
	        return d;
        }
	}
	[[nodiscard]] const AMap<AString, AJsonElement>& asObject() const;
	[[nodiscard]] const AVector<AJsonElement>& asArray() const;

	[[nodiscard]] const AJsonElement& operator[](size_t index) const;
	[[nodiscard]] const AJsonElement& operator[](const AString& key) const;

    template<typename Model>
    _<IListModel<Model>> asModelList(const AStringVector& columns) const {
        auto list = _new<AListModel<Model>>();
        auto fields = AModelMeta<Model>::getFields();
        for (AJsonElement i : asArray()) {
            auto jsonRow = i.asArray();
            Model modelRow;
            assert(columns.size() == jsonRow.size());
            for (size_t i = 0; i < columns.size(); ++i) {
                fields[columns[i]]->set(modelRow, jsonRow[i].asVariant());
            }
            list << modelRow;
        }
        return list;
    }
};

class API_AUI_JSON AJsonValue: public AJsonElement
{
public:
	AJsonValue(const AVariant& value);
};
class API_AUI_JSON AJsonObject: public AJsonElement
{
public:
	AJsonObject(const AMap<AString, AJsonElement>& value);
	AJsonObject();
    [[nodiscard]] AJsonElement& operator[](const AString& key);
    [[nodiscard]] const AJsonElement& operator[](const AString& key) const;

    auto contains(const AString& key) const {
        return asObject().contains(key);
    }

    using Iterator = AMap<AString, AJsonElement>::iterator;
    using ConstIterator = AMap<AString, AJsonElement>::const_iterator;
    using Pair = AMap<AString, AJsonElement>::value_type;
};

template<>
inline AJsonElement AVariant::to<AJsonElement>() const {
    return AJsonValue(*this);
}

class API_AUI_JSON AJsonArray: public AJsonElement
{
public:
	AJsonArray(const AVector<AJsonElement>& value);
	AJsonArray();

	bool empty() const;
	void push_back(const AJsonElement& value);
	AJsonArray& operator<<(const AJsonElement& value);

    static AJsonArray fromVariantArray(const AVector<AVariant>& value);
};