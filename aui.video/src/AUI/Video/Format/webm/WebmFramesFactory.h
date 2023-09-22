#pragma once

#include "AUI/Video/AVideoFrame.h"
#include "AUI/Image/IImageFactory.h"
#include "AUI/Video/IVideoFactory.h"

class APortionedMultipleStream;
class AAsyncVideoProcessor;

class WebmFramesFactory : public IVideoFactory {
public:
    explicit WebmFramesFactory(_<IInputStream> stream);

    AImage provideImage(const glm::ivec2& size) override;

    bool isNewImageAvailable() override;

    glm::ivec2 getSizeHint() override;

private:
    AOptional<AVideoFrame> mFrame;
    std::chrono::time_point<std::chrono::system_clock> mPlaybackStarted;
    std::chrono::time_point<std::chrono::system_clock> mLastTimeProvided;
    int64_t mLastTimecode;
    _<AAsyncVideoProcessor> mProcessor;
    _<APortionedMultipleStream> mAudioSource;
    friend class MyWebmCallback;

    void initialize();
};
