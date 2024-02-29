// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
        }
        throw;
    }
    template<typename T>
    [[nodiscard]]
    T getAxisValue(const glm::tvec2<T>& v) {
        switch (mDirection) {
            case ALayoutDirection::VERTICAL  : return v.y;
            case ALayoutDirection::HORIZONTAL: return v.x;
        }
        throw;
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


