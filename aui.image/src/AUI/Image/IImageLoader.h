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
#include "IImageFactory.h"
#include <AUI/Common/AByteBufferView.h>

class AByteBuffer;
class IDrawable;

/**
 * @brief Class-loader of abstract images that can be displayed on the screen.
 * @ingroup image
 */
class IImageLoader {
public:
    /**
     * @param buffer buffer with the raw image file contents.
     * @return true, if this IImageLoader accepts image stored in this buffer
     */
    virtual bool matches(AByteBufferView buffer) = 0;

    /**
     * @brief The drawable (vector) image loader implementation.
     * @return image factory. Can be <code>nullptr</code> if <code>getRasterImage</code> implemented.
     * @details
     * Called if and only if <code>matches</code> returned true.
     */
    virtual _<IImageFactory> getImageFactory(AByteBufferView buffer) { return nullptr; };

    /**
     * @brief The image loader implementation (raster).
     * @return raster image. Can be <code>nullptr</code> if <code>getDrawable</code> implemented.
     * @details
     * Called if and only if <code>matches</code> returned true.
     */
    virtual _<AImage> getRasterImage(AByteBufferView buffer) = 0;
};

#include "AUI/Common/AByteBuffer.h"