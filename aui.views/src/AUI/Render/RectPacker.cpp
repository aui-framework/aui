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

RectPacker::RectPacker(int width, int height) : mWidth(width), mHeight(height) { mSkyline.push_back({ 0, 0, width }); }

int RectPacker::getMinY(int index, int width, int& outMaxY) const {
    outMaxY = mSkyline[index].y;
    int remaining_width = width;

    for (size_t i = index; i < mSkyline.size(); ++i) {
        outMaxY = std::max(outMaxY, mSkyline[i].y);
        remaining_width -= mSkyline[i].width;
        if (remaining_width <= 0) {
            return outMaxY;   // fits; Y is the max of all covered nodes
        }
    }
    return -1;   // rect extends past the right edge of the atlas
}

bool RectPacker::allocateRect(Rect& r, int width, int height) {
    int best_y = mHeight + 1;
    int best_waste = mWidth + 1;   // wasted width at the best candidate
    int best_index = -1;

    for (int i = 0; i < static_cast<int>(mSkyline.size()); ++i) {
        // A rect starting here would overflow the right edge — skip.
        if (mSkyline[i].x + width > mWidth)
            continue;

        int max_y;
        int y = getMinY(i, width, max_y);
        if (y == -1)
            continue;   // does not fit horizontally
        if (y + height > mHeight)
            continue;   // does not fit vertically

        // Skyline-BL tiebreaker: prefer lowest Y, then least wasted width.
        // Wasted width = how much of the first node's width is unused by this rect.
        int waste = mSkyline[i].width - width;
        if (y < best_y || (y == best_y && waste < best_waste)) {
            best_y = y;
            best_waste = waste;
            best_index = i;
        }
    }

    if (best_index == -1)
        return false;

    r = Rect(mSkyline[best_index].x, best_y, width, height);

    // Insert the new node before removing covered nodes so best_index stays valid.
    Node new_node = { r.x, r.y + r.height, width };
    mSkyline.insert(mSkyline.begin() + best_index, new_node);

    // The inserted node sits at best_index; covered old nodes start at best_index+1.
    int remaining_width = width;
    int erase_pos = best_index + 1;
    while (remaining_width > 0 && erase_pos < static_cast<int>(mSkyline.size())) {
        if (mSkyline[erase_pos].width <= remaining_width) {
            remaining_width -= mSkyline[erase_pos].width;
            mSkyline.erase(mSkyline.begin() + erase_pos);
            // erase_pos stays the same — next node slid into this position
        } else {
            // Partially covered: shrink the surviving node from the left.
            mSkyline[erase_pos].x += remaining_width;
            mSkyline[erase_pos].width -= remaining_width;
            remaining_width = 0;
        }
    }

    // Merge adjacent nodes that share the same Y (coalesce the skyline).
    for (int i = 0; i < static_cast<int>(mSkyline.size()) - 1;) {
        if (mSkyline[i].y == mSkyline[i + 1].y) {
            mSkyline[i].width += mSkyline[i + 1].width;
            mSkyline.erase(mSkyline.begin() + i + 1);
            // do not increment i — check merged node against its new neighbour
        } else {
            ++i;
        }
    }

    return true;
}
