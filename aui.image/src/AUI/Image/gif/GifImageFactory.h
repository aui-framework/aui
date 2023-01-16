//
// Created by dervisdev on 1/12/2023.
//
#pragma once

#include "AUI/Common/AByteBufferView.h"
#include "AUI/Image/IImageFactory.h"

class GifImageFactory : public IImageFactory {
private:
    std::chrono::time_point<std::chrono::system_clock> mLastFrameStarted;
    unsigned char* mLoadedGifPixels;
    _<AImage> mCurrentFrame;
    int* mDelays;
    int mGifWidth;
    int mGifHeight;
    int mFramesCount;
    int mChannelsCount;
    int mCurrentFrameIndex;

public:
    explicit GifImageFactory(AByteBufferView buf);
    ~GifImageFactory();
    AImage provideImage(const glm::ivec2& size) override;
    bool isNewImageAvailable() override;
};