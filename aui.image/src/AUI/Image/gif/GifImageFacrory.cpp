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

//
// Created by dervisdev on 1/12/2023.
//
#include "GifImageFactory.h"
#include "AUI/Common/AByteBufferView.h"
#include "AUI/Image/AImage.h"
#include "stb_image.h"
#include "stb_image_write.h"

GifImageFactory::GifImageFactory(AByteBufferView buf) {
    mCurrentFrameIndex = 0;
    mLoadedGifPixels = stbi_load_gif_from_memory(reinterpret_cast<unsigned const char*>(buf.data()),
                                                  buf.size(),
                                                  &mDelays,
                                                  &mGifWidth,
                                                  &mGifHeight,
                                                  &mFramesCount,
                                                  &mChannelsCount,
                                                  4);
}

GifImageFactory::~GifImageFactory() {
    stbi_image_free(mLoadedGifPixels);
    delete mDelays;
}

AImage GifImageFactory::provideImage(const glm::ivec2 &size) {
    if (!isNewImageAvailable())
        return *mCurrentFrame;

    mCurrentFrameIndex++;
    if (mCurrentFrameIndex >= mFramesCount) {
        mCurrentFrameIndex = 0;
    }

    unsigned format = APixelFormat::BYTE;
    switch (mChannelsCount) {
        case 3:
            format |= APixelFormat::RGB;
            break;
        case 4:
            format |= APixelFormat::RGBA;
            break;
        default:
            assert(0);
    }


    mLastFrameStarted = std::chrono::system_clock::now();
    int frameBufferSize = mGifWidth * mGifHeight * mChannelsCount;
    int currentFrameOffset = frameBufferSize * mCurrentFrameIndex;
    mCurrentFrame = _new<AImage>(AByteBufferView(reinterpret_cast<const char*>(mLoadedGifPixels + currentFrameOffset), frameBufferSize), glm::uvec2{mGifWidth, mGifHeight}, format);
    return *mCurrentFrame;
}

bool GifImageFactory::isNewImageAvailable() {
    auto timeSinceLastFrame = std::chrono::system_clock::now() - mLastFrameStarted;
    auto millisecondsSinceFrame = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceLastFrame);
    return !mCurrentFrame || millisecondsSinceFrame.count() >= mDelays[mCurrentFrameIndex];
}

glm::ivec2 GifImageFactory::getSizeHint() {
    return { mGifWidth, mGifHeight };
}
