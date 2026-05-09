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
#include <AUI/Util/AMinMaxSizes.hpp>
#include <AUI/Enum/Visibility.h>
#include <range/v3/range.hpp>
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

    static int computePerpendicularSize(
        const auto& view,
        int ourAxisSize,
        int availablePerpendicularSize) {
        const auto minMaxSizes = view->computeMinMaxSizes(ourAxisSize);
        const int minimumPerpendicularSize = getPerpAxisValue(minMaxSizes.min);
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            if (getPerpAxisValue(view->getExpanding()) != 0 && getPerpAxisValue(view->getFixedSize()) == 0) {
                return glm::max(availablePerpendicularSize, minimumPerpendicularSize);
            }
            return glm::max(minimumPerpendicularSize, view->measure({
                .minWidth = ourAxisSize,
                .maxWidth = ourAxisSize,
                .maxHeight = availablePerpendicularSize,
            }).y);
        } else {
            if (getPerpAxisValue(view->getExpanding()) != 0 && getPerpAxisValue(view->getFixedSize()) == 0) {
                return glm::max(availablePerpendicularSize, minimumPerpendicularSize);
            }
            return glm::max(minimumPerpendicularSize, view->measure({
                .minHeight = ourAxisSize,
                .maxHeight = ourAxisSize,
                .maxWidth = availablePerpendicularSize,
            }).x);
        }
    }

    static int computeExpandingLowerBound(const auto& view, int perpendicularConstraint) {
            const auto minMaxSizes = view->computeMinMaxSizes(perpendicularConstraint);
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            return minMaxSizes.min.x;
        } else {
            return minMaxSizes.min.y;
        }
    }

    static int computePreferredMainAxisSize(const auto& view, int perpendicularConstraint) {
            const auto minMaxSizes = view->computeMinMaxSizes();
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            if (perpendicularConstraint != -1) {
                return view->measure(AConstraints::fixedHeight(perpendicularConstraint)).x;
            }
            return minMaxSizes.max.x;
        } else {
            if (perpendicularConstraint != -1) {
                return view->measure(AConstraints::fixedWidth(perpendicularConstraint)).y;
            }
            return minMaxSizes.max.y;
        }
    }

    static std::vector<MainAxisSizeInfo> resolveMainAxisSizes(
        ranges::range auto&& views,
        int spacing,
        int containerAxisSize,
        auto&& preferredMainAxisProvider,
        auto&& minimumMainAxisProvider) {

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
                info.baseSize = minimumMainAxisProvider(view);
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
            },
            [&](const auto& view) {
                const int childPerpAxis = glm::max(
                    0,
                    getPerpAxisValue(paddedSize - view->getMargin().occupiedSize()));
                return computeExpandingLowerBound(view, childPerpAxis);
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

    static int preferredWidth(ranges::range auto&& views, int spacing, int height) {
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            return preferredOurAxis(views, spacing, height);
        } else {
            return preferredPerpendicularAxis(views, spacing, height);
        }
    }

    static int preferredHeight(ranges::range auto&& views, int spacing, int width) {
        if constexpr (direction == ALayoutDirection::VERTICAL) {
            return preferredOurAxis(views, spacing, width);
        } else {
            return preferredPerpendicularAxis(views, spacing, width);
        }
    }

    static int measurePerpendicularForMainAxisSize(const auto& view, int ourAxisSize) {
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            return view->measure({
                .minWidth = ourAxisSize,
                .maxWidth = ourAxisSize,
            }).y;
        } else {
            return view->measure({
                .minHeight = ourAxisSize,
                .maxHeight = ourAxisSize,
            }).x;
        }
    }

    static int computeMinimumPerpendicularContribution(const auto& view, const AMinMaxSizes& minMax, glm::ivec2 margin) {
        const int minimumPerpendicularSize = getPerpAxisValue(minMax.min) + getPerpAxisValue(margin);
        const int minimumMainAxisSize = getAxisValue(minMax.min);
        if (minimumMainAxisSize <= 0) {
            return minimumPerpendicularSize;
        }
        return glm::max(
            minimumPerpendicularSize,
            measurePerpendicularForMainAxisSize(view, minimumMainAxisSize) + getPerpAxisValue(margin));
    }

    static int computeMaximumPerpendicularContribution(const auto& view, const AMinMaxSizes& minMax, glm::ivec2 margin) {
        const int maximumPerpendicularSize = getPerpAxisValue(minMax.max) + getPerpAxisValue(margin);
        const int maximumMainAxisSize = getAxisValue(minMax.max);
        if (maximumMainAxisSize <= 0) {
            return maximumPerpendicularSize;
        }
        return glm::max(
            maximumPerpendicularSize,
            measurePerpendicularForMainAxisSize(view, maximumMainAxisSize) + getPerpAxisValue(margin));
    }

    static AMinMaxSizes computeIntrinsicMinMaxSizes(ranges::range auto&& views, int spacing) {
        AMinMaxSizes result;
        int visibleCount = 0;

        for (const auto& view : views) {
            if (!(view->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
                continue;
            }

            const auto minMax = view->computeMinMaxSizes();
            const auto margin = view->getMargin().occupiedSize();
            const auto childMin = minMax.min + margin;
            const auto childMax = minMax.max + margin;
            const int childMeasuredMinPerp = computeMinimumPerpendicularContribution(view, minMax, margin);
            const int childMeasuredMaxPerp = computeMaximumPerpendicularContribution(view, minMax, margin);

            if constexpr (direction == ALayoutDirection::HORIZONTAL) {
                result.min.x += childMin.x;
                result.max.x += childMax.x;
                result.min.y = glm::max(result.min.y, childMeasuredMinPerp);
                result.max.y = glm::max(result.max.y, childMeasuredMaxPerp);
            } else {
                result.min.x = glm::max(result.min.x, childMeasuredMinPerp);
                result.max.x = glm::max(result.max.x, childMeasuredMaxPerp);
                result.min.y += childMin.y;
                result.max.y += childMax.y;
            }

            ++visibleCount;
        }

        const int totalSpacing = glm::max(0, visibleCount - 1) * spacing;
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            result.min.x += totalSpacing;
            result.max.x += totalSpacing;
        } else {
            result.min.y += totalSpacing;
            result.max.y += totalSpacing;
        }

        return result;
    }

  static glm::ivec2 onIntrinsicMeasure(ranges::range auto&& views, int spacing, AConstraints constraints) {
      if constexpr (direction == ALayoutDirection::HORIZONTAL) {
        int height_constraint = constraints.maxHeight >= 1000000 ? -1 : constraints.maxHeight;
        int width = preferredWidth(views, spacing, height_constraint);

        int width_constraint_for_height = constraints.maxWidth >= 1000000
            ? glm::max(width, constraints.minWidth)
            : glm::clamp(width, constraints.minWidth, constraints.maxWidth);
        int height = preferredHeight(views, spacing, width_constraint_for_height);
        return { width, height };
      } else {
        int width_constraint = constraints.maxWidth >= 1000000 ? -1 : constraints.maxWidth;
        int height = preferredHeight(views, spacing, width_constraint);

        int height_constraint_for_width = constraints.maxHeight >= 1000000
            ? glm::max(height, constraints.minHeight)
            : glm::clamp(height, constraints.minHeight, constraints.maxHeight);
        int width = preferredWidth(views, spacing, height_constraint_for_width);
        return { width, height };
      }
    }

private:
    static int preferredMainAxisSize(const auto& view) {
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            return view->computeMinMaxSizes().max.x;
        } else {
            return view->computeMinMaxSizes().max.y;
        }
    }

    static int preferredOurAxis(ranges::range auto&& views, int spacing, int perpAxisConstraint) {
        int sum = -spacing;
        for (const auto& v : views) {
            if (!(v->getVisibility() & Visibility::FLAG_CONSUME_SPACE))
                continue;
            const int childPerpAxisConstraint =
                (perpAxisConstraint == -1) ? -1 : std::max(0, perpAxisConstraint - getPerpAxisValue(v->getMargin().occupiedSize()));

            int childOurAxisIntrinsic = 0;
            if constexpr (direction == ALayoutDirection::HORIZONTAL) {
                childOurAxisIntrinsic = childPerpAxisConstraint == -1
                    ? v->computeMinMaxSizes().max.x
                    : v->measure(AConstraints::fixedHeight(childPerpAxisConstraint)).x;
            } else {
                childOurAxisIntrinsic = childPerpAxisConstraint == -1
                    ? v->computeMinMaxSizes().max.y
                    : v->measure(AConstraints::fixedWidth(childPerpAxisConstraint)).y;
            }

            sum += childOurAxisIntrinsic + getAxisValue(v->getMargin().occupiedSize()) + spacing;
        }
        return std::max(0, sum);
    }

    static int preferredPerpendicularAxis(ranges::range auto&& views, int spacing, int ourAxisConstraint) {
        if (ourAxisConstraint == -1) {
            int maxPerp = 0;
            for (const auto& v : views) {
                if (!(v->getVisibility() & Visibility::FLAG_CONSUME_SPACE))
                    continue;

                int childPerpAxisIntrinsic;
                if constexpr (direction == ALayoutDirection::HORIZONTAL) {
                    childPerpAxisIntrinsic = v->computeMinMaxSizes().max.y;
                } else {
                    childPerpAxisIntrinsic = v->computeMinMaxSizes().max.x;
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
                return preferredMainAxisSize(view);
            }, [&](const auto& view) {
                return computeExpandingLowerBound(view, -1);
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
