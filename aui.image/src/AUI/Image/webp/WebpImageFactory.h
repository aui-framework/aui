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

#include "AUI/Image/IAnimatedImageFactory.h"
#include <webp/mux_types.h>

struct WebPAnimDecoder;

/**
 * Passed webp must have animation
 */
class WebpImageFactory : public IAnimatedImageFactory {
public:
    explicit WebpImageFactory(AByteBufferView buffer);

    ~WebpImageFactory();

    AImage provideImage(const glm::ivec2& size) override;

    bool isNewImageAvailable() override;

    glm::ivec2 getSizeHint() override;

    bool hasAnimationFinished() override;

    enum class FrameSkipMode {
        PAUSE,         // 0: pause, continue from the same frame
        SKIP_FRAMES,   // 1: skip frames and loops (fast forward)
        CATCH_UP       // 2: catch up one frame per call
    };

    void setFrameSkipMode(const FrameSkipMode mode) { mSkipMode = mode;}
    FrameSkipMode getFrameSkipMode() const { return mSkipMode; }

private:
    size_t mWidth;
    size_t mHeight;

    /**
     * mCurrentFrame will be equal to 0 after the first invoke of loadNextFrame()
     */
    size_t mCurrentFrame = -1;
    size_t mFrameCount;
    AVector<std::chrono::milliseconds> mDurations;
    size_t mLoopsPassed = 0;
    size_t mLoopCount;
    bool mAnimationFinished = false;
    std::chrono::time_point<std::chrono::system_clock> mLastTimeFrameStarted;

    FrameSkipMode mSkipMode = FrameSkipMode::PAUSE;
    std::chrono::milliseconds mTotalDuration = std::chrono::milliseconds(0);

    static constexpr APixelFormat PIXEL_FORMAT = APixelFormat(APixelFormat::RGBA_BYTE);

    void loadNextFrame();

    WebPData mFileData;
    WebPAnimDecoder* mDecoder = nullptr;
    uint8_t* mDecodedFrameBuffer = nullptr;
    int mDecodedFrameTimestamp = 0;
};
