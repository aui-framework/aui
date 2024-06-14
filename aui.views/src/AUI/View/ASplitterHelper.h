/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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
private:
    ALayoutDirection mDirection; // will be initialized in the Builder
    size_t mDraggingDividerIndex = -1;
    int mDragOffset; // may be uninitialized
    AVector<_<AView>> mItems;

    float getTotalOccupiedSizeOf(const _<AView>& view) {
        return mDirection == ALayoutDirection::VERTICAL ? view->getTotalOccupiedHeight() : view->getTotalOccupiedWidth();
    }

    template<typename T>
    [[nodiscard]]
    T& getAxisValue(glm::tvec2<T>& v) {
        switch (mDirection) {
            case ALayoutDirection::VERTICAL  : return v.y;
            case ALayoutDirection::HORIZONTAL: return v.x;
            default: throw AException("invalid direction");
        }
    }
    template<typename T>
    [[nodiscard]]
    T getAxisValue(const glm::tvec2<T>& v) {
        switch (mDirection) {
            case ALayoutDirection::VERTICAL  : return v.y;
            case ALayoutDirection::HORIZONTAL: return v.x;
            default: throw AException("invalid direction");
        }
    }


public:
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

    void setItems(AVector<_<AView>>&& items) {
        mItems = items;
    }

    [[nodiscard]]
    bool isDragging() const {
        return mDraggingDividerIndex != -1;
    }

};


