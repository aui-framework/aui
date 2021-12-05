//
// Created by Alex2772 on 12/5/2021.
//

#pragma once


struct ViewAssertionEmpty {
    bool operator()(const _<AView>&) {
        return false;
    }
};

using empty = ViewAssertionEmpty;