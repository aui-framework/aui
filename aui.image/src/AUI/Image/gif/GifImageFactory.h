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

//
// Created by dervisdev on 1/12/2023.
//
#pragma once

#include <AUI/Common/AByteBufferView.h>
#include <AUI/Image/IAnimatedImageFactory.h>
#include <chrono>

typedef struct nsgif nsgif_t;

class GifImageFactory : public IAnimatedImageFactory {
public:
    explicit GifImageFactory(AByteBufferView buf);

    ~GifImageFactory();

    AImage provideImage(const glm::ivec2& size) override;

    bool isNewImageAvailable() override;

    glm::ivec2 getSizeHint() override;

    bool hasAnimationFinished() override;

    ARect<int> getDirtyRect() override;

    void prepareNextFrame() override;

    uint32_t getCurrentFrameLength() override;

private:
    AByteBuffer mGifData;
    std::chrono::time_point<std::chrono::high_resolution_clock> mLastFrameStarted;
    size_t mCurrentFrameIndex = -1;
    size_t mFrameCount;
    size_t mWidth = 0;
    size_t mHeight = 0;
    uint32_t mCurrentFrameLength = 0;
    bool mAnimationFinished = false;
    ARect<int> mDirtyRect;
    nsgif_t* mContext;
    uint8_t* mLastFrameBuffer = nullptr;

    bool mIsPrepared = false;
    uint32_t mNextFrameDelay;
    uint32_t mNextFrameIndex;

    AImage fetchImage();
};