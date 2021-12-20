//
// Created by Alex2772 on 12/5/2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include <AUI/Common/ASide.h>

namespace uitest::impl {
    template<typename L, typename R>
    struct both: L, R {
        bool operator()(const _<AView>& v) {
            return L::operator()(v) && R::operator()(v);
        }
    };
}
