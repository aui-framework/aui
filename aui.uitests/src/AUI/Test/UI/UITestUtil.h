#pragma once

#include <AUI/api.h>
#include <AUI/View/AView.h>

namespace uitest {
    API_AUI_UITESTS void frame();

    namespace impl {
        template<int axisIndex>
        inline int axisAlignedDistanceBetween(const _<AView>& v1, const _<AView>& v2) {
            auto begin1 = v1->getPositionInWindow()[axisIndex];
            auto begin2 = v2->getPositionInWindow()[axisIndex];
            auto end1   = begin1 + v1->getSize()[axisIndex];
            auto end2   = begin2 + v2->getSize()[axisIndex];

            return glm::min(glm::clamp(begin2, begin1, end1) - begin2,
                            glm::clamp(end2, begin1, end1) - end2);
        }
    }

    inline int horizontalDistanceBetween(const _<AView>& v1, const _<AView>& v2) {
        return impl::axisAlignedDistanceBetween<0>(v1, v2);
    }
    inline int verticalDistanceBetween(const _<AView>& v1, const _<AView>& v2) {
        return impl::axisAlignedDistanceBetween<1>(v1, v2);
    }
}