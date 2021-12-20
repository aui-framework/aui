//
// Created by Alex2772 on 12/5/2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include <AUI/Common/ASide.h>

namespace UITest::impl {
    template<ASide side>
    struct side_value {};

    template<>
    struct side_value<ASide::LEFT> {
        int operator()(const _<AView>& view) { return view->getPositionInWindow().x; }
    };

    template<>
    struct side_value<ASide::TOP> {
        int operator()(const _<AView>& view) { return view->getPositionInWindow().y; }
    };

    template<>
    struct side_value<ASide::RIGHT> {
        int operator()(const _<AView>& view) { return view->getPositionInWindow().x + view->getSize().x; }
    };

    template<>
    struct side_value<ASide::BOTTOM> {
        int operator()(const _<AView>& view) { return view->getPositionInWindow().y + view->getSize().y; }
    };


    template<ASide side>
    struct align {
        std::optional<int> value;

        bool operator()(const _<AView>& v) {
            int current = side_value<side>{}(v);
            if (!value) {
                value = current;
                return true;
            }
            return current == *value;
        }
    };

}

using leftAligned = UITest::impl::align<ASide::LEFT>;
using topAligned = UITest::impl::align<ASide::TOP>;
using rightAligned = UITest::impl::align<ASide::RIGHT>;
using bottomAligned = UITest::impl::align<ASide::BOTTOM>;