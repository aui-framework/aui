//
// Created by Alex2772 on 12/5/2021.
//

#pragma once


struct ViewAssertionGone {
    bool operator()(const _<AView>& v) {
        return v->getVisibility() == Visibility::GONE;
    }
};

using gone = ViewAssertionGone;