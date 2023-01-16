// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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

//
// Created by Alex2772 on 11/11/2021.
//

#include "ASplitterHelper.h"

void ASplitterHelper::beginDrag(const glm::ivec2& mousePos) {
    int cursor = getAxisValue(mousePos);
    size_t dividerIndex = 0;
    for (auto& v : mItems) {
        if (getAxisValue(v->getPosition()) + getAxisValue(v->getSize()) / 2 > cursor) {
            break;
        }
        dividerIndex += 1;
    }
    mDraggingDividerIndex = dividerIndex - 1;

    if (isDragging()) {
        mDragOffset = getAxisValue(mousePos);
    }
}

bool ASplitterHelper::mouseDrag(const glm::ivec2& mousePos) {
    if (isDragging()) {
        int newDragOffset = getAxisValue(mousePos);

        // positive delta = movement of the slider to right, negative delta = movement of the slider to left
        int delta = newDragOffset - mDragOffset;

        if (delta == 0) {
            // zero delta? do nothing (because zero breaks glm::sign which is used below)
            return false;
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

            const auto prevSize = currentItem->getFixedSize();
            currentItem->setFixedSize({0, 0});

            // check if current view can handle us all free space
            int minSize = mDirection == ALayoutDirection::VERTICAL
                          ? currentItem->getMinimumHeight()
                          : currentItem->getMinimumWidth();
            int currentSize = getAxisValue(currentItem->getSize());
            int currentDelta = currentSize - minSize;
            if (currentDelta >= amountToShrink) {
                // best case. current view handled all free space
                glm::ivec2 fixedSize = prevSize;
                getAxisValue(fixedSize) = currentSize - amountToShrink;
                currentItem->setFixedSize(fixedSize);
                amountToShrink = 0;
                break;
            } else if (currentDelta != 0) {
                // worse case. current view partially handled free space, so we have to spread it to the next elements
                glm::ivec2 fixedSize = prevSize;
                getAxisValue(fixedSize) = currentSize - currentDelta;
                currentItem->setFixedSize(fixedSize);
                amountToShrink -= currentDelta;
            }
            currentItem->setExpanding(false);
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

                glm::ivec2 fixedSize = currentItem->getFixedSize();
                currentItem->setFixedSize({0, 0});

                // check if current view can handle us all free space
                int maxSize = getAxisValue(currentItem->getMaxSize());
                int currentSize = getAxisValue(currentItem->getSize());
                int currentDelta = maxSize - currentSize;

                if (currentDelta >= amountToEnlarge) {
                    // best case. current view handled all free space
                    getAxisValue(fixedSize) = currentSize + amountToEnlarge;
                    currentItem->setFixedSize(fixedSize);
                    amountToEnlarge = 0;
                    break;
                } else if (currentDelta != 0) {
                    // worse case. current view partially handled free space, so we have to spread it to the next elements
                    getAxisValue(fixedSize) = currentSize + currentDelta;
                    currentItem->setFixedSize(fixedSize);
                    amountToEnlarge -= currentDelta;
                }
                currentItem->setExpanding(false);
            }
        }
        mDragOffset += amountToEnlargeCopy * glm::sign(delta);
        return true;
    }
    return false;
}

