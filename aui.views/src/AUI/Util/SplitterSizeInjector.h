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
    ASPLITTER_IMPL_FORWARD_METHOD(getExpanding)
    ASPLITTER_IMPL_FORWARD_METHOD(getVisibility)
    ASPLITTER_IMPL_FORWARD_METHOD(getMargin)
    ASPLITTER_IMPL_FORWARD_METHOD(layout)
    ASPLITTER_IMPL_FORWARD_METHOD(getSize)
    ASPLITTER_IMPL_FORWARD_METHOD(getFixedSize)
    ASPLITTER_IMPL_FORWARD_METHOD(getMaxSize)
    ASPLITTER_IMPL_FORWARD_METHOD(getMinSize)

    glm::ivec2 measure(AConstraints constraints) const {
        return item.view->measure(constraints);
    }

    int computeIntrinsicWidth(int height) const {
        auto value = item.view->computeIntrinsicWidth(height);
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            if (item.overridedSize) {
                value = glm::max(value, *item.overridedSize);
            }
        }
        return value;
    }

    int computeWidth(int height) const {
        auto value = item.view->computeWidth(height);
        if constexpr (direction == ALayoutDirection::HORIZONTAL) {
            if (item.overridedSize) {
                value = glm::max(value, *item.overridedSize);
            }
        }
        return value;
    }

    int computeIntrinsicHeight(int width) const {
        auto value = item.view->computeIntrinsicHeight(width);
        if constexpr (direction == ALayoutDirection::VERTICAL) {
            if (item.overridedSize) {
                value = glm::max(value, *item.overridedSize);
            }
        }
        return value;
    }

    int computeHeight(int width) const {
        auto value = item.view->computeHeight(width);
        if constexpr (direction == ALayoutDirection::VERTICAL) {
            if (item.overridedSize) {
                value = glm::max(value, *item.overridedSize);
            }
        }
        return value;
    }

    glm::ivec2 getMinimumSize() const {
        auto size = item.view->getMinimumSize();
        if (item.overridedSize) {
            auto& value = aui::layout_direction::getAxisValue(direction, size);
            value = glm::max(*item.overridedSize, value);
        }
        return size;
    }

    bool operator==(const SizeInjector& rhs) const noexcept { return item.view == rhs.item.view; }
};
