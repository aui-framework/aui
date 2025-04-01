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
#include "AUI/Common/SharedPtr.h"
#include "AImage.h"

/**
 * @brief Produces images by the specified size.
 */
class IImageFactory
{
public:
    virtual AImage provideImage(const glm::ivec2& size) = 0;
    virtual bool isNewImageAvailable() { return true; }

    /**
     * @see <a href="IDrawable::getSizeHint">IDrawable::getSizeHint</a>
     */
    virtual glm::ivec2 getSizeHint() {
        return {0, 0};
    }
};
