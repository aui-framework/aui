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
#include <AUI/Util/AConstraints.hpp>
#include <AUI/Enum/Visibility.h>
#include <vector>

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
private:
    struct MainAxisSizeInfo {
        bool visible = false;
        int weight = 0;
        int baseSize = 0;
        int maxSize = std::numeric_limits<int>::max();
        int finalSize = 0;
    };

    static bool isExpanding(const MainAxisSizeInfo& info) {
        return info.visible && info.weight > 0 && info.finalSize < info.maxSize;
    }

    static void distributeRemainingSpace(std::vector<MainAxisSizeInfo>& sizes, int remainingSpace) {
        std::vector<size_t> expanding;
        expanding.reserve(sizes.size());

        for (size_t i = 0; i < sizes.size(); ++i) {
            if (isExpanding(sizes[i])) {
                expanding.push_back(i);
            }
        }

        while (remainingSpace > 0 && !expanding.empty()) {
            int totalWeight = 0;
            for (auto index : expanding) {
                totalWeight += sizes[index].weight;
            }

            if (totalWeight <= 0) {
                break;
            }

            const int spaceToDistribute = remainingSpace;
            int distributed = 0;
            int distributedShares = 0;

            for (size_t i = 0; i < expanding.size();) {
                auto& info = sizes[expanding[i]];

                const int share = i + 1 == expanding.size()
                    ? spaceToDistribute - distributedShares
                    : spaceToDistribute * info.weight / totalWeight;

                distributedShares += share;

                const int capacity = info.maxSize - info.finalSize;
                const int extra = glm::min(share, capacity);

                info.finalSize += extra;
                distributed += extra;

                if (info.finalSize >= info.maxSize) {
                    expanding.erase(expanding.begin() + static_cast<ptrdiff_t>(i));
                } else {
                    ++i;
                }
            }

            if (distributed <= 0) {
                break;
            }

            remainingSpace -= distributed;
        }
    }

public:
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

    static int computePerpendicularSize(const auto& view, int ourAxisSize, int availablePerpendicularSize) {
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            if (getPerpAxisValue(view->getExpanding()) != 0 && getPerpAxisValue(view->getFixedSize()) == 0) {
                return availablePerpendicularSize;
            }
            return glm::min(availablePerpendicularSize, view->measure({
                .minWidth = ourAxisSize,
                .maxWidth = ourAxisSize,
                .maxHeight = availablePerpendicularSize,
            }).y);
        } else {
            if (getPerpAxisValue(view->getExpanding()) != 0 && getPerpAxisValue(view->getFixedSize()) == 0) {
                return availablePerpendicularSize;
            }
            return glm::min(availablePerpendicularSize, view->measure({
                .minHeight = ourAxisSize,
                .maxHeight = ourAxisSize,
                .maxWidth = availablePerpendicularSize,
            }).x);
        }
    }

    static int computeExpandingLowerBound(const auto& view) {
        return getAxisValue(view->getMinSize());
    }

    static int computePreferredMainAxisSize(const auto& view, int perpendicularConstraint) {
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            return view->computeWidth(perpendicularConstraint);
        } else {
            return view->computeHeight(perpendicularConstraint);
        }
    }

    static std::vector<MainAxisSizeInfo> resolveMainAxisSizes(
        ranges::range auto&& views,
        int spacing,
        int containerAxisSize,
        auto&& preferredMainAxisProvider) {

        std::vector<MainAxisSizeInfo> result;
        result.reserve(static_cast<size_t>(ranges::size(views)));

        int remainingSpace = containerAxisSize;
        int visibleCount = 0;

        for (const auto& view : views) {
            MainAxisSizeInfo info;

            if (!(view->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
                result.push_back(info);
                continue;
            }

            info.visible = true;

            const int fixedSize = getAxisValue(view->getFixedSize());
            const int expanding = getAxisValue(view->getExpanding());
            const int rawMaxSize = getAxisValue(view->getMaxSize());

            if (fixedSize > 0) {
                info.baseSize = fixedSize;
                info.maxSize = fixedSize;
            } else if (expanding > 0) {
                info.weight = expanding;
                info.baseSize = computeExpandingLowerBound(view);
                info.maxSize = rawMaxSize > 0
                    ? glm::max(info.baseSize, rawMaxSize)
                    : std::numeric_limits<int>::max();
            } else {
                info.baseSize = preferredMainAxisProvider(view);
                info.maxSize = rawMaxSize > 0
                    ? glm::max(info.baseSize, rawMaxSize)
                    : std::numeric_limits<int>::max();
            }

            info.finalSize = info.baseSize;

            remainingSpace -= info.baseSize;
            remainingSpace -= getAxisValue(view->getMargin().occupiedSize());

            ++visibleCount;
            result.push_back(info);
        }

        remainingSpace -= glm::max(0, visibleCount - 1) * spacing;

        if (remainingSpace > 0) {
            distributeRemainingSpace(result, remainingSpace);
        }

        return result;
    }

    static void layout(glm::ivec2 paddedPosition, glm::ivec2 paddedSize, ranges::range auto&& views, int spacing) {
        static constexpr auto FIXED_POINT_DENOMINATOR = 2 << 4;

        if (views.empty()) {
            return;
        }

        for (const auto& view : views) {
            view->ensureAssUpdated();
        }

        const auto resolvedMainAxisSizes = resolveMainAxisSizes(
            views,
            spacing,
            getAxisValue(paddedSize),
            [&](const auto& view) {
                const int childPerpAxis = glm::max(
                    0,
                    getPerpAxisValue(paddedSize - view->getMargin().occupiedSize()));
                return computePreferredMainAxisSize(view, childPerpAxis);
            });

        long long posOurAxis = getAxisValue(paddedPosition) * FIXED_POINT_DENOMINATOR;
        size_t index = 0;

        for (const auto& view : views) {
            const auto& resolvedSize = resolvedMainAxisSizes[index++];

            if (!resolvedSize.visible) {
                continue;
            }

            const auto margins = view->getMargin();
            const auto viewMaxSize = view->getMaxSize();

            const int viewPosOurAxis =
                static_cast<int>(posOurAxis / FIXED_POINT_DENOMINATOR) + getAxisValue(margins.leftTop());

            const int viewPosPerpAxis =
                getPerpAxisValue(paddedPosition + margins.leftTop());

            const int viewSizeOurAxis = resolvedSize.finalSize;

            const int availablePerpendicularSize = glm::min(
                getPerpAxisValue(paddedSize - margins.occupiedSize()),
                getPerpAxisValue(viewMaxSize));

            const int viewSizePerpAxis = computePerpendicularSize(
                view,
                viewSizeOurAxis,
                availablePerpendicularSize);

            view->layout(
                getAxisValue(glm::ivec2 { viewPosOurAxis, viewPosPerpAxis }),
                getAxisValue(glm::ivec2 { viewPosPerpAxis, viewPosOurAxis }),
                getAxisValue(glm::ivec2 { viewSizeOurAxis, viewSizePerpAxis }),
                getAxisValue(glm::ivec2 { viewSizePerpAxis, viewSizeOurAxis }));

            const int occupiedOurAxisSize = viewSizeOurAxis + getAxisValue(margins.occupiedSize());

            posOurAxis += (spacing + occupiedOurAxisSize) * FIXED_POINT_DENOMINATOR;
        }
    }

    static int onComputeIntrinsicWidth(ranges::range auto&& views, int spacing, int height) {
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            return onComputeIntrinsicOurAxis(views, spacing, height);
        } else {
            return onComputeIntrinsicPerpAxis(views, spacing, height);
        }
    }

    static int onComputeIntrinsicHeight(ranges::range auto&& views, int spacing, int width) {
        if constexpr (direction == ALayoutDirection::VERTICAL) {
            return onComputeIntrinsicOurAxis(views, spacing, width);
        } else {
            return onComputeIntrinsicPerpAxis(views, spacing, width);
        }
    }

    static glm::ivec2 onIntrinsicMeasure(ranges::range auto&& views, int spacing, AConstraints constraints) {
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            int heightConstraint = constraints.maxHeight >= 1000000 ? -1 : constraints.maxHeight;
            int width = onComputeIntrinsicWidth(views, spacing, heightConstraint);
            int heightConstraintForHeight = constraints.maxWidth >= 1000000 ? width : glm::clamp(width, constraints.minWidth, constraints.maxWidth);
            int height = onComputeIntrinsicHeight(views, spacing, heightConstraintForHeight);
            return { width, height };
        } else {
            int widthConstraint = constraints.maxWidth >= 1000000 ? -1 : constraints.maxWidth;
            int height = onComputeIntrinsicHeight(views, spacing, widthConstraint);
            int widthConstraintForWidth = constraints.maxHeight >= 1000000 ? height : glm::clamp(height, constraints.minHeight, constraints.maxHeight);
            int width = onComputeIntrinsicWidth(views, spacing, widthConstraintForWidth);
            return { width, height };
        }
    }

private:
    static int computeMainAxisSizeForConstraint(const auto& view) {
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            return view->computeWidth(-1);
        } else {
            return view->computeHeight(-1);
        }
    }

    static int onComputeIntrinsicOurAxis(ranges::range auto&& views, int spacing, int perpAxisConstraint) {
        int sum = -spacing;
        for (const auto& v : views) {
            if (!(v->getVisibility() & Visibility::FLAG_CONSUME_SPACE))
                continue;
            int childPerpAxisConstraint = (perpAxisConstraint == -1) ? -1 : std::max(0, perpAxisConstraint - getPerpAxisValue(v->getMargin().occupiedSize()));

            int childOurAxisIntrinsic;
            if constexpr (direction == ALayoutDirection::HORIZONTAL) {
                childOurAxisIntrinsic = v->computeWidth(childPerpAxisConstraint);
            } else {
                childOurAxisIntrinsic = v->computeHeight(childPerpAxisConstraint);
            }

            sum += childOurAxisIntrinsic + getAxisValue(v->getMargin().occupiedSize()) + spacing;
        }
        return std::max(0, sum);
    }

    static int onComputeIntrinsicPerpAxis(ranges::range auto&& views, int spacing, int ourAxisConstraint) {
        if (ourAxisConstraint == -1) {
            int maxPerp = 0;
            for (const auto& v : views) {
                if (!(v->getVisibility() & Visibility::FLAG_CONSUME_SPACE))
                    continue;

                int childPerpAxisIntrinsic;
                if constexpr (direction == ALayoutDirection::HORIZONTAL) {
                    childPerpAxisIntrinsic = v->computeHeight(-1);
                } else {
                    childPerpAxisIntrinsic = v->computeWidth(-1);
                }

                maxPerp = std::max(maxPerp, childPerpAxisIntrinsic + getPerpAxisValue(v->getMargin().occupiedSize()));
            }
            return maxPerp;
        }

        for (const auto& v : views) {
            v->ensureAssUpdated();
        }
        const auto resolvedMainAxisSizes =
            resolveMainAxisSizes(views, spacing, ourAxisConstraint, [&](const auto& view) {
                return computeMainAxisSizeForConstraint(view);
            });

        int maxPerp = 0;
        size_t index = 0;
        for (const auto& v : views) {
            const auto& resolvedSize = resolvedMainAxisSizes[index++];
            if (!(v->getVisibility() & Visibility::FLAG_CONSUME_SPACE))
                continue;

            int childPerpAxisIntrinsic;
            if constexpr (direction == ALayoutDirection::HORIZONTAL) {
                childPerpAxisIntrinsic = v->measure({
                    .minWidth = resolvedSize.finalSize,
                    .maxWidth = resolvedSize.finalSize,
                }).y;
            } else {
                childPerpAxisIntrinsic = v->measure({
                    .minHeight = resolvedSize.finalSize,
                    .maxHeight = resolvedSize.finalSize,
                }).x;
            }

            maxPerp = std::max(maxPerp, childPerpAxisIntrinsic + getPerpAxisValue(v->getMargin().occupiedSize()));
        }
        return maxPerp;
    }
};
}   // namespace aui
