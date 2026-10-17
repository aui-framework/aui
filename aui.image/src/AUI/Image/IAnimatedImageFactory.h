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

#include <AUI/Geometry2D/ARect.h>
#include "AUI/Image/IImageFactory.h"

/**
 * @brief Produces images related to an animation
 */
class IAnimatedImageFactory : public IImageFactory {
public:

    /**
     * @brief Returns true, if last provided frame was last (within one cycle of animation)
     */
    virtual bool hasAnimationFinished() = 0;

    /**
     * @return dirty rect of the last provided frame
     */
    virtual ARect<int> getDirtyRect() = 0;

    /**
     * @brief Prepares metadata for the next frame (dirty rect, delay).
     */
    virtual void prepareNextFrame() = 0;

    /**
     * @return delay for the current (last provided) frame in milliseconds.
     */
    virtual uint32_t getCurrentFrameLength() = 0;
};
