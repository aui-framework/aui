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
    auto timeSinceLastFrame = std::chrono::system_clock::now() - mLastFrameStarted;
    auto millisecondsSinceFrame = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceLastFrame);
    if (mCurrentFrame && millisecondsSinceFrame.count() < mDelays[mCurrentFrameIndex])
        return *mCurrentFrame;

    mCurrentFrameIndex++;
    if (mCurrentFrameIndex >= mFramesCount)
        mCurrentFrameIndex = 0;

    unsigned format = AImageFormat::BYTE;
    switch (mChannelsCount) {
        case 3:
            format |= AImageFormat::RGB;
            break;
        case 4:
            format |= AImageFormat::RGBA;
            break;
        default:
            assert(0);
    }

    mLastFrameStarted = std::chrono::system_clock::now();
    int frameBufferSize = mGifWidth * mGifHeight * mChannelsCount;
    int currentFrameOffset = frameBufferSize * mCurrentFrameIndex;
    AByteBuffer buffer(reinterpret_cast<const char*>(mLoadedGifPixels + currentFrameOffset), frameBufferSize);
    mCurrentFrame = _new<AImage>(buffer, mGifWidth, mGifHeight, format);
    return *mCurrentFrame;
}
