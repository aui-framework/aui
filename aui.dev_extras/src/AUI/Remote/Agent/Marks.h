/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "AUI/Common/AVector.h"
#include "AUI/Geometry2D/ARect.h"
#include "AUI/Image/AImage.h"

namespace aui::remote::marks {

/**
 * @brief A numbered box drawn over a screenshot.
 */
struct Mark {
    int label;

    /**
     * @brief The box in the pixels of the image.
     */
    ARect<int> rect;
};

/**
 * @brief Draws the boxes and their numbers over the image (set-of-mark).
 * @details
 * Each mark gets a color from a palette, so neighbouring marks are told apart. A number is drawn in white on a plate of
 * the mark's color with a dark outline, which keeps it readable on any background. A plate goes to a corner of its box
 * (outside above it if there is room) where it covers neither another plate nor, if possible, another box. All the
 * plates are drawn after all the boxes.
 *
 * Draws with a built-in bitmap font of digits: no renderer is involved, so the window, the frame stream and plain
 * screenshots are unaffected.
 *
 * @param pixelsPerDp image pixels per density-independent pixel; lines and digits are sized in dp, so they keep up with
 *        the text of the application at any DPI.
 */
API_AUI_DEV_EXTRAS void draw(AImage& image, const AVector<Mark>& marks, float pixelsPerDp);

}   // namespace aui::remote::marks
