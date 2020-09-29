#pragma once

#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/Common/AVector.h>
#include "AUI/Common/AVariant.h"
#include "AJsonElement.h"
#include <AUI/IO/IOutputStream.h>

class IJsonElement {
public:
	virtual ~IJsonElement() = default;

	virtual bool isVariant() = 0;
	virtual bool isObject() = 0;
	virtual bool isArray() = 0;
	virtual bool isNull() {
	    return false;
	}

	[[nodiscard]] virtual AVariant& asVariant() = 0;
	[[nodiscard]] virtual AMap<AString, AJsonElement>& asObject() = 0;
	[[nodiscard]] virtual AVector<AJsonElement>& asArray() = 0;
    virtual void serialize(const _<IOutputStream>& os) const = 0;
};