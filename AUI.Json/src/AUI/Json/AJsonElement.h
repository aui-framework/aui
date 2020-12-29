#pragma once

#include <AUI/Json.h>
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

	[[nodiscard]] const AVariant& asVariant() const;
	[[nodiscard]] int asInt() const;
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

    bool contains(const AString& key) const;

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

	void push_back(const AJsonElement& value);
	AJsonArray& operator<<(const AJsonElement& value);

    static AJsonArray fromVariantArray(const AVector<AVariant>& value);
};