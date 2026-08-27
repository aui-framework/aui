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

#pragma once

#include <vector>
#include <AUI/Common/SharedPtr.h>

namespace aui {
/**
 * @brief A rectangle in the atlas.
 */
struct Rect {
    int x, y, width, height;

    Rect(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}
    Rect() : x(0), y(0), width(0), height(0) {}
};

/**
 * @brief Skyline Bottom-Left rectangle packing algorithm.
 */
class API_AUI_VIEWS RectPacker {
public:
    RectPacker(int width, int height);

    /**
     * @brief Attempt to pack a rectangle.
     * @return true if packed, false otherwise.
     */
    bool allocateRect(Rect& r, int width, int height);

private:
    struct Node {
        int x, y, width;
    };

    int mWidth, mHeight;
    std::vector<Node> mSkyline;

    int getMinY(int index, int width, int& outMaxY) const;
};
}
