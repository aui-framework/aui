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

#include <AUI/View/ASplitterHelper.h>

/**
 * @brief AView-like object that forwards some methods from AView and injects overridedSize if set.
 */
template<ALayoutDirection direction>
struct SizeInjector {
    const ASplitterHelper::Item& item;
    auto operator->() const { return this; }

#define ASPLITTER_IMPL_FORWARD_METHOD(name)                  \
    template <typename... Args>                              \
    auto name(Args&&... args) const {                        \
        return item.view->name(std::forward<Args>(args)...); \
    }
    ASPLITTER_IMPL_FORWARD_METHOD(ensureAssUpdated)
    ASPLITTER_IMPL_FORWARD_METHOD(getVisibility)
    ASPLITTER_IMPL_FORWARD_METHOD(getMargin)
    ASPLITTER_IMPL_FORWARD_METHOD(layout)
    ASPLITTER_IMPL_FORWARD_METHOD(getSize)
    ASPLITTER_IMPL_FORWARD_METHOD(getFixedSize)
    ASPLITTER_IMPL_FORWARD_METHOD(getMaxSize)
    ASPLITTER_IMPL_FORWARD_METHOD(getMinSize)

    glm::ivec2 measure(AConstraints constraints) const {
        if (item.overridedSize) {
            const int overridedAxis = glm::max(
                aui::layout_direction::getAxisValue(direction, item.view->getMinSize()),
                *item.overridedSize);
            if constexpr (direction == ALayoutDirection::HORIZONTAL) {
                constraints.minWidth = overridedAxis;
                constraints.maxWidth = overridedAxis;
            } else {
                constraints.minHeight = overridedAxis;
                constraints.maxHeight = overridedAxis;
            }
        }
        return item.view->measure(constraints);
    }

    glm::ivec2 getExpanding() const {
        auto expanding = item.view->getExpanding();
        if (item.overridedSize) {
            auto& value = aui::layout_direction::getAxisValue(direction, expanding);
            value = 0;
        }
        return expanding;
    }

    glm::ivec2 getMinSize() const {
        return item.view->getMinSize();
    }

    AMinMaxSizes computeMinMaxSizes(int height = -1) const {
        auto value = item.view->computeMinMaxSizes(height);
        if (item.overridedSize) {
            auto& minAxis = aui::layout_direction::getAxisValue(direction, value.min);
            auto& maxAxis = aui::layout_direction::getAxisValue(direction, value.max);
            const int overridedAxis = glm::max(aui::layout_direction::getAxisValue(direction, item.view->getMinSize()),
                                               *item.overridedSize);
            minAxis = overridedAxis;
            maxAxis = overridedAxis;
        }
        return value;
    }

    bool operator==(const SizeInjector& rhs) const noexcept { return item.view == rhs.item.view; }
};
