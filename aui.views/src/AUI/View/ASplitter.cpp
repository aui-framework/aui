/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "ASplitter.h"
#include <AUI/Util/UIBuildingHelpers.h>

ASplitter::ASplitter() {
}

void ASplitter::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    AViewContainer::onMousePressed(pos, button);
    if (isDragging()) {
        auto& divider = mDividers[mDraggingDividerIndex];
        mDragOffset = getAxisValue(pos);
    }
}


void ASplitter::setSize(int width, int height) {
    AViewContainer::setSize(width, height);
}

void ASplitter::onMouseMove(glm::ivec2 pos) {
    AViewContainer::onMouseMove(pos);
    if (isDragging()) {
        int newDragOffset = getAxisValue(pos);
        int delta = mDragOffset - newDragOffset;
        int originalDelta = delta;

        auto& firstItem             = mItems[mDraggingDividerIndex];
        auto& secondItem            = mItems[mDraggingDividerIndex + 1];

        auto firstItemSize          = firstItem->getSize();
        auto secondItemSize         = secondItem->getSize();

        auto& firstItemAxisValue    = getAxisValue(firstItemSize);
        auto& secondItemAxisValue   = getAxisValue(secondItemSize);

        // reset fixedSize because it affects getMinimumWidth/Height
        firstItem->setFixedSize({0, 0});
        secondItem->setFixedSize({0, 0});

        auto firstItemMinAxisValue  = mDirection == LayoutDirection::VERTICAL ? firstItem->getMinimumHeight()  : firstItem->getMinimumWidth();
        auto secondItemMinAxisValue = mDirection == LayoutDirection::VERTICAL ? secondItem->getMinimumHeight() : secondItem->getMinimumWidth();

        auto firstItemMaxAxisValue  = getAxisValue(firstItem->getMaxSize());
        auto secondItemMaxAxisValue = getAxisValue(secondItem->getMaxSize());

        // clamp delta to keep minSize/maxSize
        delta = firstItemAxisValue - glm::clamp(firstItemAxisValue - delta, firstItemMinAxisValue, firstItemMaxAxisValue);
        delta = glm::clamp(secondItemAxisValue + delta, secondItemMinAxisValue, secondItemMaxAxisValue) - secondItemAxisValue;

        firstItemAxisValue -= delta;
        secondItemAxisValue += delta;

        firstItem->setFixedSize(firstItemSize);
        secondItem->setFixedSize(secondItemSize);

        mDragOffset = newDragOffset + (originalDelta - delta);

        updateLayout();
        redraw();
    }
}

void ASplitter::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AViewContainer::onMouseReleased(pos, button);
    mDraggingDividerIndex = -1;
}

void ASplitter::updateSplitterItems() {
    if (mDirection == LayoutDirection::VERTICAL) {
        setLayout(_new<AVerticalLayout>());
    } else {
        setLayout(_new<AHorizontalLayout>());
    }

    unsigned dividerIndex = 0;
    for (auto& item : mItems) {
        if (dividerIndex > 0) {
            auto divider = mDividerFactory().connect(&AView::mousePressed, this, [this, index = dividerIndex - 1] {
                mDraggingDividerIndex = index;
            });
            divider->setMouseCollisionPolicy(MouseCollisionPolicy::MARGIN); // extends click area
            addView(divider);
            mDividers << divider;
        }
        addView(item);
        ++dividerIndex;
    }
    float axisWholeSize = mDirection == LayoutDirection::VERTICAL ? getContentHeight() : getContentWidth();

    // subtract dividers' size from axisWholeSize
    for (auto& divider : mDividers) {
        axisWholeSize -= getTotalOccupiedSizeOf(divider);
    }

    for (auto& item : mItems) {
        auto size = getTotalOccupiedSizeOf(item);
    }
}
