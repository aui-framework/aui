//
// Created by dervisdev on 1/26/2023.
//

#pragma once

#include <AUI/Platform/ACursor.h>
#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

struct ScrollbarAppearance {
public:
    enum AxisValue {
        NO_SCROLL_SHOW_CONTENT,
        NO_SCROLL_HIDE_CONTENT,

        VISIBLE,
        INVISIBLE,
        GONE
    };

    AxisValue getVertical() {
        return mVertical;
    }

    AxisValue getHorizontal() {
        return mHorizontal;
    }

    ScrollbarAppearance() = default;
    explicit ScrollbarAppearance(AxisValue both) : mVertical(both), mHorizontal(both) {};
    ScrollbarAppearance(AxisValue vertical, AxisValue horizontal) : mVertical(vertical), mHorizontal(horizontal) {};

private:
    AxisValue mVertical = AxisValue::VISIBLE;
    AxisValue mHorizontal = AxisValue::VISIBLE;
};

namespace ass {
    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<ScrollbarAppearance>: IDeclarationBase {
        private:
            ScrollbarAppearance mInfo;

        public:
            Declaration(const ScrollbarAppearance& info) : mInfo(info) { }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}