/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Util/ALayoutDirection.h>

namespace aui {

/**
 * @brief Shared implementation of AVerticalLayout and AHorizontalLayout.
 * @details
 * HVLayout does not strictly requires to layout AView. The only requirement is to pass range of items that implement
 * methods required by HVLayout (as AView does). This make compile time polymorphism possible.
 *
 * ASplitter is an example of object that requires AHorizontalLayout/AVerticalLayout-like behaviour with some changes.
 */
template <ALayoutDirection direction>
struct HVLayout {
    /**
     * @brief On direction == HORIZONTAL returns x; on direction == VERTICAL returns y
     */
    template <typename T>
    [[nodiscard]]
    static T& getAxisValue(glm::tvec2<T>& v) {
        return aui::layout_direction::getAxisValue(direction, v);
    }

    /**
     * @brief On direction == HORIZONTAL returns x; on direction == VERTICAL returns y
     */
    template <typename T>
    [[nodiscard]]
    static T getAxisValue(const glm::tvec2<T>& v) {
        return aui::layout_direction::getAxisValue(direction, v);
    }

    /**
     * @brief On direction == HORIZONTAL returns y; on direction == VERTICAL returns x
     */
    template <typename T>
    [[nodiscard]]
    static T& getPerpAxisValue(glm::tvec2<T>& v) {
        return aui::layout_direction::getPerpendicularAxisValue(direction, v);
    }

    /**
     * @brief On direction == HORIZONTAL returns y; on direction == VERTICAL returns x
     */
    template <typename T>
    [[nodiscard]]
    static T getPerpAxisValue(const glm::tvec2<T>& v) {
        return aui::layout_direction::getPerpendicularAxisValue(direction, v);
    }

    static void onResize(glm::ivec2 paddedPosition, glm::ivec2 paddedSize, ranges::range auto&& views, int spacing) {
        static constexpr auto FIXED_POINT_DENOMINATOR = 2 << 4;

        if (views.empty())
            return;

        int sum = 0;
        int availableSpaceForExpandingViews = getAxisValue(paddedSize) + spacing;

        // first phase: calculate sum and availableSpaceForExpandingViews
        for (const auto& view : views) {
            view->ensureAssUpdated();
            if (!(view->getVisibility() & Visibility::FLAG_CONSUME_SPACE))
                continue;
            int expanding = getAxisValue(view->getExpanding());
            int minSpace = getAxisValue(view->getMinimumSize());
            sum += expanding;
            if (expanding == 0 || getAxisValue(view->getFixedSize()) != 0)   // expanding view is fixed size is equal to
                                                                             // non-expanding
                availableSpaceForExpandingViews -= minSpace + getAxisValue(view->getMargin().occupiedSize()) + spacing;
            else
                availableSpaceForExpandingViews -= getAxisValue(view->getMargin().occupiedSize()) + spacing;
        }

        bool containsExpandingItems = sum > 0;

        sum = glm::max(sum, 1);

        // second phase: validate availableSpaceForExpanding for expanding views with min size and max size
        if (containsExpandingItems) {
            for (const auto& view : views) {
                if (!(view->getVisibility() & Visibility::FLAG_CONSUME_SPACE))
                    continue;

                int expanding = getAxisValue(view->getExpanding());

                if (expanding == 0 || getAxisValue(view->getFixedSize()) != 0)   // expanding view is fixed size is
                                                                                 // equal to non-expanding
                    continue;

                int spaceAcquiredByExpanding = availableSpaceForExpandingViews * expanding / sum;
                int viewMinSize = getAxisValue(view->getMinimumSize());
                auto viewMaxSize = view->getMaxSize();
                int validatedSpace = glm::clamp(spaceAcquiredByExpanding, viewMinSize, getAxisValue(viewMaxSize));
                availableSpaceForExpandingViews += (spaceAcquiredByExpanding - validatedSpace) * sum / expanding;
            }
        }

        // third phase: apply layout to views
        long long posOurAxis = getAxisValue(paddedPosition) * FIXED_POINT_DENOMINATOR;
        const auto& last = views.back();
        for (const auto& view : views) {
            if (!(view->getVisibility() & Visibility::FLAG_CONSUME_SPACE))
                continue;
            auto margins = view->getMargin();
            auto viewMaxSize = view->getMaxSize();

            int viewPosOurAxis = posOurAxis / FIXED_POINT_DENOMINATOR + getAxisValue(margins.leftTop());
            int viewPosPerpAxis = getPerpAxisValue(paddedPosition + margins.leftTop());

            if (containsExpandingItems && view == last) {
                // the last element should stick right to the border.
                int viewSizeOurAxis =
                    getAxisValue(paddedSize) - viewPosOurAxis - getAxisValue(margins.rightBottom()) +
                    getAxisValue(paddedPosition);
                int viewSizePerpAxis = getPerpAxisValue(paddedSize - margins.occupiedSize());

                view->setGeometry(
                    getAxisValue(glm::ivec2 { viewPosOurAxis, viewPosPerpAxis }),
                    getAxisValue(glm::ivec2 { viewPosPerpAxis, viewPosOurAxis }),
                    getAxisValue(glm::ivec2 { viewSizeOurAxis, viewSizePerpAxis }),
                    getAxisValue(glm::ivec2 { viewSizePerpAxis, viewSizeOurAxis }));
            } else {
                int expanding = getAxisValue(view->getExpanding());
                int viewMinSize = getAxisValue(view->getMinimumSize());
                long long viewEndPos =
                    viewPosOurAxis * FIXED_POINT_DENOMINATOR +
                    glm::clamp(
                        FIXED_POINT_DENOMINATOR * availableSpaceForExpandingViews * expanding / sum,
                        FIXED_POINT_DENOMINATOR * viewMinSize, FIXED_POINT_DENOMINATOR * getAxisValue(viewMaxSize));

                int viewSizeOurAxis = viewEndPos / FIXED_POINT_DENOMINATOR - posOurAxis / FIXED_POINT_DENOMINATOR;
                int viewSizePerpAxis =
                    glm::min(getPerpAxisValue(paddedSize - margins.occupiedSize()), getPerpAxisValue(viewMaxSize));

                view->setGeometry(
                    getAxisValue(glm::ivec2 { viewPosOurAxis, viewPosPerpAxis }),
                    getAxisValue(glm::ivec2 { viewPosPerpAxis, viewPosOurAxis }),
                    getAxisValue(glm::ivec2 { viewSizeOurAxis, viewSizePerpAxis }),
                    getAxisValue(glm::ivec2 { viewSizePerpAxis, viewSizeOurAxis }));

                if (getAxisValue(view->getSize()) == viewSizeOurAxis) {
                    posOurAxis = viewEndPos + (spacing + getAxisValue(margins.rightBottom())) * FIXED_POINT_DENOMINATOR;
                } else {
                    // view has rejected the size we have provided, and we should adopt.
                    // unfortunately, we lose the fractional part of the size.
                    posOurAxis += (spacing + getAxisValue(view->getSize() + margins.occupiedSize())) * FIXED_POINT_DENOMINATOR;
                }
                availableSpaceForExpandingViews += viewSizeOurAxis - getAxisValue(view->getSize());
            }
        }
    }

    static int getMinimumWidth(ranges::range auto&& views, int spacing) {
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            return getMinimumSizeOurAxis(views, spacing);
        }
        if constexpr (direction == ALayoutDirection::VERTICAL) {
            return getMinimumSizePerpAxis(views, spacing);
        }
    }

    static int getMinimumHeight(ranges::range auto&& views, int spacing) {
        if constexpr (direction == ALayoutDirection::VERTICAL) {
            return getMinimumSizeOurAxis(views, spacing);
        }
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            return getMinimumSizePerpAxis(views, spacing);
        }
    }

private:
    static int getMinimumSizeOurAxis(ranges::range auto&& views, int spacing) {
        int minSize = -spacing;

        for (const auto& v : views) {
            if (!(v->getVisibility() & Visibility::FLAG_CONSUME_SPACE))
                continue;
            minSize += getAxisValue(v->getMinimumSize() + v->getMargin().occupiedSize()) + spacing;
        }

        return glm::max(minSize, 0);
    }

    static int getMinimumSizePerpAxis(ranges::range auto&& views, int spacing) {
        int minSize = 0;
        for (const auto& v : views) {
            if (!(v->getVisibility() & Visibility::FLAG_CONSUME_SPACE))
                continue;
            auto h = getPerpAxisValue(v->getMinimumSize() + +v->getMargin().occupiedSize());
            minSize = glm::max(minSize, int(h));
        }
        return minSize;
    }
};
}   // namespace aui