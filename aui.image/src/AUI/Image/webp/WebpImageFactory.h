// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "AUI/Image/IAnimatedImageFactory.h"
#include <webp/mux_types.h>

struct WebPAnimDecoder;

/**
 * @note Passed webp must have animation
 */
class WebpImageFactory : public IAnimatedImageFactory {
public:
    explicit WebpImageFactory(AByteBufferView buffer);

    ~WebpImageFactory();

    AImage provideImage(const glm::ivec2& size) override;

    bool isNewImageAvailable() override;

    glm::ivec2 getSizeHint() override;

    bool hasAnimationFinished() override;

private:
    size_t mWidth;
    size_t mHeight;

    /**
     * @note mCurrentFrame will be equal to 0 after the first invoke of loadNextFrame()
     */
    size_t mCurrentFrame = -1;
    size_t mFrameCount;
    AVector<int> mDurations;
    size_t mLoopsPassed = 0;
    size_t mLoopCount;
    bool mAnimationFinished = false;
    std::chrono::time_point<std::chrono::system_clock> mLastTimeFrameStarted;

    static constexpr APixelFormat PIXEL_FORMAT = APixelFormat(APixelFormat::RGBA_BYTE);

    void loadNextFrame();

    WebPData mFileData;
    WebPAnimDecoder* mDecoder = nullptr;
    uint8_t* mDecodedFrameBuffer = nullptr;
    int mDecodedFrameTimestamp = 0;
};
