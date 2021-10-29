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

        // positive delta = movement of the slider to right, negative delta = movement of the slider to left
        int delta = newDragOffset - mDragOffset;

        if (delta == 0) {
            // zero delta? do nothing (because zero breaks glm::sign which is used below)
            return;
        }

        // our first task is shrink items to get free space.
        // we should take space from the first item after divider, then from the second item after divider, etc...

        // we'll use this value in for loops for adding it to index
        int direction = glm::sign(delta);

        int amountToShrink = glm::abs(delta);
        for (int i = int(mDraggingDividerIndex) + glm::clamp(direction, 0, 1); // first index to shrink
             i < mItems.size() && i >= 0; // dual check for both forward and backward iteration
             i += direction) {
            auto& currentItem = mItems[i];

            currentItem->setFixedSize({0, 0});

            // check if current view can handle us all free space
            int minSize = mDirection == LayoutDirection::VERTICAL
                          ? currentItem->getMinimumHeight()
                          : currentItem->getMinimumWidth();
            int currentSize = getAxisValue(currentItem->getSize());
            int currentDelta = currentSize - minSize;
            if (currentDelta >= amountToShrink) {
                // best case. current view handled all free space
                glm::ivec2 fixedSize = {0, 0};
                getAxisValue(fixedSize) = currentSize - amountToShrink;
                currentItem->setFixedSize(fixedSize);
                amountToShrink = 0;
                break;
            } else if (currentDelta != 0) {
                // worse case. current view partially handled free space, so we have to spread it to the next elements
                glm::ivec2 fixedSize = {0, 0};
                getAxisValue(fixedSize) = currentSize - currentDelta;
                currentItem->setFixedSize(fixedSize);
                amountToShrink -= currentDelta;
            }
        }

        int amountToEnlarge = (glm::abs(delta) - amountToShrink);
        int amountToEnlargeCopy = amountToEnlarge;
        if (amountToEnlarge != 0) {
            // now we'll move backwards in order to increase the size of items.
            direction = -direction;
            for (int i = int(mDraggingDividerIndex) + glm::clamp(direction, 0, 1); // first index to shrink
                 i < mItems.size() && i >= 0; // dual check for both forward and backward iteration
                 i += direction) {
                auto& currentItem = mItems[i];
                // same stuff as above, but we'll check for maxSize

                currentItem->setFixedSize({0, 0});

                // check if current view can handle us all free space
                int maxSize = getAxisValue(currentItem->getMaxSize());
                int currentSize = getAxisValue(currentItem->getSize());
                int currentDelta = maxSize - currentSize;

                if (currentDelta >= amountToEnlarge) {
                    // best case. current view handled all free space
                    glm::ivec2 fixedSize = {0, 0};
                    getAxisValue(fixedSize) = currentSize + amountToEnlarge;
                    currentItem->setFixedSize(fixedSize);
                    amountToEnlarge = 0;
                    break;
                } else if (currentDelta != 0) {
                    // worse case. current view partially handled free space, so we have to spread it to the next elements
                    glm::ivec2 fixedSize = {0, 0};
                    getAxisValue(fixedSize) = currentSize + currentDelta;
                    currentItem->setFixedSize(fixedSize);
                    amountToEnlarge -= currentDelta;
                }
            }
        }
        mDragOffset += amountToEnlargeCopy * glm::sign(delta);

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

    bool atLeastOneItemHasExpanding = false;
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
        atLeastOneItemHasExpanding |= getAxisValue(item->getExpanding()) > 0;
        ++dividerIndex;
    }
    if (!atLeastOneItemHasExpanding) {
        auto spacer = _new<ASpacer>();
        addView(spacer);
        mItems << spacer;
    }
}
