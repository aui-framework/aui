//
// Created by Alex2772 on 11/11/2021.
//

#include "ASplitterHelper.h"

void ASplitterHelper::beginDrag(const glm::ivec2& mousePos) {
    int cursor = getAxisValue(mousePos);
    size_t dividerIndex = 0;
    for (auto& v : mItems) {
        if (getAxisValue(v->getPosition()) + getAxisValue(v->getSize()) > cursor) {
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
        return true;
    }
    return false;
}

