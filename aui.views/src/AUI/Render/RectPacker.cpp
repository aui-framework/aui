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

#include "RectPacker.hpp"

#include <algorithm>

using namespace aui;

RectPacker::RectPacker(int width, int height) : mWidth(width), mHeight(height) {
    mSkyline.push_back({ 0, 0, width });
}

int RectPacker::getMinY(int index, int width, int& outMaxY) const {
    int x = mSkyline[index].x;
    int y = mSkyline[index].y;
    int remainingWidth = width;
    outMaxY = y;

    for (size_t i = index; i < mSkyline.size(); ++i) {
        outMaxY = std::max(outMaxY, mSkyline[i].y);
        remainingWidth -= mSkyline[i].width;
        if (remainingWidth <= 0) {
            return outMaxY;
        }
    }
    return -1; // Should not happen if width is within limits
}

bool RectPacker::allocateRect(Rect& r, int width, int height) {
    int bestY = mHeight + 1;
    int bestWidth = mWidth + 1;
    int bestIndex = -1;

    for (int i = 0; i < (int)mSkyline.size(); ++i) {
        int maxY;
        int y = getMinY(i, width, maxY);
        if (y != -1 && y + height <= mHeight) {
            if (y < bestY || (y == bestY && mSkyline[i].width < bestWidth)) {
                bestY = y;
                bestIndex = i;
                bestWidth = mSkyline[i].width;
            }
        }
    }

    if (bestIndex == -1) return false;

    r = Rect(mSkyline[bestIndex].x, bestY, width, height);

    // Update skyline
    Node newNode = { r.x, r.y + r.height, width };
    
    // Remove nodes that are fully covered by the new node
    int remainingWidth = width;
    while (remainingWidth > 0) {
        if (mSkyline[bestIndex].width <= remainingWidth) {
            remainingWidth -= mSkyline[bestIndex].width;
            mSkyline.erase(mSkyline.begin() + bestIndex);
        } else {
            mSkyline[bestIndex].x += remainingWidth;
            mSkyline[bestIndex].width -= remainingWidth;
            remainingWidth = 0;
        }
    }
    
    mSkyline.insert(mSkyline.begin() + bestIndex, newNode);

    // Merge adjacent nodes with same height
    for (int i = 0; i < (int)mSkyline.size() - 1; ++i) {
        if (mSkyline[i].y == mSkyline[i + 1].y) {
            mSkyline[i].width += mSkyline[i + 1].width;
            mSkyline.erase(mSkyline.begin() + i + 1);
            --i;
        }
    }

    return true;
}
