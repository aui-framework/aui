//
// Created by Alex2772 on 12/5/2021.
//

#pragma once

#include <AUI/Util/AMetric.h>


template<int I>
struct ViewAssertionSize {
    int value;
    explicit ViewAssertionSize(const AMetric& value): value(value.getValuePx()) {}

    bool operator()(const _<AView>& v) {
        return v->getSize()[I] == value;
    }
};

template<int I>
struct ViewAssertionSizeSame {
    AOptional<int> value;

    bool operator()(const _<AView>& v) {
        if (!value) {
            value = v->getSize()[I];
            return true;
        }
        return v->getSize()[I] == *value;
    }
};

template<int I>
struct ViewAssertionSizeIsMinimal {
    AOptional<int> value;

    bool operator()(const _<AView>& v) {
        return v->getSize()[I] == v->getMinimumSize()[I];
    }
};

using width = ViewAssertionSize<0>;
using height = ViewAssertionSize<1>;

using widthIsMinimal = ViewAssertionSizeIsMinimal<0>;
using heightIsMinimal = ViewAssertionSizeIsMinimal<1>;

using sameWidth = ViewAssertionSizeSame<0>;
using sameHeight = ViewAssertionSizeSame<1>;