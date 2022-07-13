//
// Created by Alex2772 on 12/5/2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include <AUI/Common/ASide.h>
#include "Traits.h"

namespace uitest::impl {
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

    template<ASide side>
    struct less_of {
        UIMatcher matcher;

        less_of(UIMatcher matcher) : matcher(std::move(matcher)) {}

        bool operator()(const _<AView>& lhs) {
            auto views = matcher.toSet();
            return std::all_of(views.begin(), views.end(), [&](const _<AView>& rhs) {
                return side_value<side>{}(lhs) < side_value<side>{}(rhs);
            });
        }
    };

    template<ASide side>
    struct greater_of {
        UIMatcher matcher;

        greater_of(UIMatcher matcher) : matcher(std::move(matcher)) {}

        bool operator()(const _<AView>& lhs) {
            auto views = matcher.toSet();
            return std::all_of(views.begin(), views.end(), [&](const _<AView>& rhs) {
                return side_value<side>{}(lhs) > side_value<side>{}(rhs);
            });
        }
    };
}

using leftAligned = uitest::impl::align<ASide::LEFT>;
using topAligned = uitest::impl::align<ASide::TOP>;
using rightAligned = uitest::impl::align<ASide::RIGHT>;
using bottomAligned = uitest::impl::align<ASide::BOTTOM>;


using bottomAboveBottomOf = uitest::impl::less_of<ASide::BOTTOM>;
using bottomBelowBottomOf = uitest::impl::greater_of<ASide::BOTTOM>;


using leftRightAligned = uitest::impl::both<uitest::impl::align<ASide::LEFT>, uitest::impl::align<ASide::RIGHT>>;
using topBottomAligned = uitest::impl::both<uitest::impl::align<ASide::TOP>, uitest::impl::align<ASide::BOTTOM>>;
