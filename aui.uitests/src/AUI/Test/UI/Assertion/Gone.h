//
// Created by Alex2772 on 12/5/2021.
//

#pragma once


struct ViewAssertionGone {
    using IGNORE_VISIBILITY = std::true_type;

    bool operator()(const _<AView>& v) {
        return v->getVisibility() == Visibility::GONE;
    }
};

using gone = ViewAssertionGone;