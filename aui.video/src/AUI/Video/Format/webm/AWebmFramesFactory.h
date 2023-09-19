#pragma once

#include "AUI/Video/AFrame.h"
#include "AUI/Image/IImageFactory.h"
#include "AUI/Util/APimpl.h"
#include "AUI/Thread/AFuture.h"

class AAsyncVideoProcessor;

class AWebmFramesFactory : public IImageFactory {
public:
    explicit AWebmFramesFactory(_<IInputStream> stream);

    AImage provideImage(const glm::ivec2& size) override;

    bool isNewImageAvailable() override;

    glm::ivec2 getSizeHint() override;

private:
    AOptional<AFrame> mFrame;
    std::chrono::time_point<std::chrono::system_clock> mPlaybackStarted;
    std::chrono::time_point<std::chrono::system_clock> mLastTimeProvided;
    int64_t mLastTimecode;
    _<AAsyncVideoProcessor> mProcessor;
    friend class MyWebmCallback;
};
