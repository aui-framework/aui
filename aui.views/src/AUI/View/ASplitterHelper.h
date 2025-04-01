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
#include <AUI/View/AView.h>

class ASplitterHelper {
    friend class ASplitter;
public:
    struct Item {
        _<AView> view;
        AOptional<int> overridedSize;
    };

    ASplitterHelper() = default;
    ASplitterHelper(ALayoutDirection direction) : mDirection(direction) {}

    void setDirection(ALayoutDirection direction) {
        mDirection = direction;
    }

    void beginDrag(const glm::ivec2& mousePos);
    bool mouseDrag(const glm::ivec2& mousePos);
    void endDrag() {
        mDraggingDividerIndex = -1;
    }

    [[nodiscard]]
    const AVector<Item>& items() const { return mItems; }

    void setItems(AVector<Item> items) {
        mItems = std::move(items);
    }

    void setItems(const AVector<_<AView>>& items) {
        mItems = items.map([](const _<AView>& view) { return Item { .view = view }; });
    }

    [[nodiscard]]
    bool isDragging() const {
        return mDraggingDividerIndex != -1;
    }

    bool isDraggingArea(glm::ivec2 position);

    /**
     * @brief Reclaims space via specified divider index.
     * @param space space to reclaim in px. Cannot be zero.
     * @param dividerIndex divider index.
     * @return space argument but reduced (abs) by amount of space successfully reclaimed.
     * @details
     * Negative space reclaims space from the left direction of dividerIndex, positive space reclaims space from the
     * right direction of dividerIndex.
     */
    int reclaimSpace(int space, size_t dividerIndex);

    /**
     * @brief Reclaims space via the last divider.
     * @param space space to reclaim in px. Cannot be zero.
     * @return space argument but reduced (abs) by amount of space successfully reclaimed.
     * @details
     * Negative space reclaims space from the left direction of dividerIndex, positive space reclaims space from the
     * right direction of dividerIndex.
     */
    int reclaimSpace(int space) {
        return reclaimSpace(space, mItems.size() - 1);
    }

private:
    ALayoutDirection mDirection; // will be initialized in the Builder
    size_t mDraggingDividerIndex = -1;
    int mDragOffset; // may be uninitialized
    AVector<Item> mItems;

    float getTotalOccupiedSizeOf(const _<AView>& view) {
        return mDirection == ALayoutDirection::VERTICAL ? view->getTotalOccupiedHeight() : view->getTotalOccupiedWidth();
    }

    template<typename T>
    [[nodiscard]]
    T& getAxisValue(glm::tvec2<T>& v) {
        return aui::layout_direction::getAxisValue(mDirection, v);
    }

    template<typename T>
    [[nodiscard]]
    T getAxisValue(const glm::tvec2<T>& v) {
        return aui::layout_direction::getAxisValue(mDirection, v);
    }

};


