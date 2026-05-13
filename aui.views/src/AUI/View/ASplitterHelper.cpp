/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by Alex2772 on 11/11/2021.
//

#include "ASplitterHelper.h"

static constexpr auto CLICK_BIAS = 4_dp;

AOptional<size_t> ASplitterHelper::dividerIndexAt(glm::ivec2 position) const {
  if (mItems.empty()) {
    return std::nullopt;
  }
  int cursor = getAxisValue(position);
  size_t dividerIndex = 0;
  for (const auto& v : mItems) {
    if (getAxisValue(v.view->getPosition()) + getAxisValue(v.view->getSize()) / 2 > cursor) {
      break;
    }
    dividerIndex += 1;
  }
  if (dividerIndex == 0 || dividerIndex >= mItems.size()) {
    return std::nullopt;
  }
  return dividerIndex - 1;
}

void ASplitterHelper::beginDrag(const glm::ivec2& mousePos) {
  mDraggingDividerIndex = dividerIndexAt(mousePos).valueOr(size_t(-1));
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
    auto amountToShrink = reclaimSpace(delta, mDraggingDividerIndex);
    int direction = glm::sign(delta);
    int amountToEnlarge = (glm::abs(delta) - amountToShrink);
    int amountToEnlargeCopy = amountToEnlarge;
    if (amountToEnlarge != 0) {
      // now we'll move backwards in order to increase the size of items.
      direction = -direction;

      auto applyEnlargement = [&](Item& currentItem) {
        // same stuff as above, but we'll check for maxSize

        const int currentSize = getEffectiveSize(currentItem);

        // check if current view can handle us all free space
        const int maxSize = getAxisValue(currentItem.view->getMaxSize());
        const int currentDelta = maxSize == -1 ? amountToEnlarge : glm::max(0, maxSize - currentSize);

        if (currentDelta >= amountToEnlarge) {
          // best case. current view handled all free space
          currentItem.overridedSize = currentSize + amountToEnlarge;
          currentItem.view->requestLayout();
          amountToEnlarge = 0;
        } else if (currentDelta != 0) {
          // worse case. current view partially handled free space, so we have to spread it to the next elements
          currentItem.overridedSize = currentSize + currentDelta;
          currentItem.view->requestLayout();
          amountToEnlarge -= currentDelta;
        }
      };

      // prioritize expanding views
      for (int i = int(mDraggingDividerIndex) + glm::clamp(direction, 0, 1);   // first index to shrink
           i < mItems.size() && i >= 0 && amountToEnlarge != 0;   // dual check for both forward and backward iteration
           i += direction) {
        auto& currentItem = mItems[i];
        if (getAxisValue(currentItem.view->getExpanding()) == 0) {
          continue;
        }
        applyEnlargement(currentItem);
      }

      // apply enlargement for non-expanding views if anything remaining in amountToEnlarge
      for (int i = int(mDraggingDividerIndex) + glm::clamp(direction, 0, 1);   // first index to shrink
           i < mItems.size() && i >= 0 && amountToEnlarge != 0;   // dual check for both forward and backward iteration
           i += direction) {
        auto& currentItem = mItems[i];
        if (getAxisValue(currentItem.view->getExpanding()) != 0) {
          continue;
        }
        applyEnlargement(currentItem);
      }
    }
    mDragOffset += amountToEnlargeCopy * glm::sign(delta);
    return true;
  }
  return false;
}

bool ASplitterHelper::isDraggingArea(glm::ivec2 position) {
  if (mItems.empty()) {
    return false;
  }
  if (getAxisValue(position) <= getAxisValue(mItems.first().view->getPosition()) + CLICK_BIAS.getValuePx()) {
    // position is before first view.
    return false;
  }
  if (getAxisValue(position) >=
      getAxisValue(mItems.last().view->getPosition() + mItems.last().view->getSize()) - CLICK_BIAS.getValuePx() * 2) {
    // position is after last view.
    return false;
  }

  for (auto& v : mItems) {
    auto viewPos = getAxisValue(v.view->getPosition()) + CLICK_BIAS.getValuePx();
    auto viewSize = getAxisValue(v.view->getSize()) - CLICK_BIAS.getValuePx() * 2.f;

    if (getAxisValue(position) > viewPos && getAxisValue(position) < viewPos + viewSize) {
      return false;
    }

    if (getAxisValue(position) <= viewPos) {
      break;
    }
  }
  return true;
}
int ASplitterHelper::reclaimSpace(int space, size_t dividerIndex) {
  AUI_ASSERT(space != 0);

  // we'll use this value in for loops for adding it to index
  int direction = glm::sign(space);

  int amountToShrink = glm::abs(space);
  int firstIndexToShrink = int(dividerIndex) + glm::clamp(direction, 0, 1);
  if (firstIndexToShrink >= int(mItems.size())) {
    firstIndexToShrink = int(mItems.size()) - 1;
  }
  for (int i = firstIndexToShrink;    // first index to shrink
       i < mItems.size() && i >= 0;   // dual check for both forward and backward iteration
       i += direction) {
    auto& currentItem = mItems[i];

    const int currentSize = getEffectiveSize(currentItem);

    // check if current view can handle us all free space
    const int minSize = getAxisValue(currentItem.view->getMinSize());
    const int currentDelta = glm::max(0, currentSize - minSize);
    if (currentDelta >= amountToShrink) {
      // best case. current view handled all free space
      currentItem.overridedSize = currentSize - amountToShrink;
      currentItem.view->requestLayout();
      amountToShrink = 0;
      break;
    } else if (currentDelta != 0) {
      // worse case. current view partially handled free space, so we have to spread it to the next elements
      currentItem.overridedSize = currentSize - currentDelta;
      currentItem.view->requestLayout();
      amountToShrink -= currentDelta;
    }
  }

  return amountToShrink;
}
