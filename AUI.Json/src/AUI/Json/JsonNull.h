#pragma once


#include "IJsonElement.h"

class JsonNull: public IJsonElement {
public:
    ~JsonNull() override;

    bool isVariant() override;
    bool isObject() override;
    bool isArray() override;
    bool isNull() override;

    AVariant& asVariant() override;
    AMap<AString, AJsonElement>& asObject() override;
    AVector<AJsonElement>& asArray() override;

    void serialize(const _<IOutputStream>& os) const override;
};

