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

#include "AUI/Image/IImageFactory.h"

struct WebPBitstreamFeatures;

/**
 * @note Passed webp must have animation
 */
class WebpImageFactory : public IImageFactory {
public:
    explicit WebpImageFactory(AByteBufferView buffer);
    WebpImageFactory(AByteBufferView buffer, const WebPBitstreamFeatures& features);

    AImage provideImage(const glm::ivec2& size) override;

    bool isNewImageAvailable() override;

    glm::ivec2 getSizeHint() override;

private:
    int mWidth;
    int mHeight;
    int mFormat;

    size_t mCurrentFrame = 0;
    AVector<AByteBuffer> mFrames;
    AVector<int> mDurations;
    APixelFormat mPixelFormat = APixelFormat(0);
    size_t mLoopCount;
    size_t mLoopsPassed = 0;
    std::chrono::time_point<std::chrono::system_clock> mLastTimeFrameStarted;

    void loadFeatures(const WebPBitstreamFeatures& features);

    void loadFrames(AByteBufferView buffer);
};
