#include "WebmFramesFactory.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Video/AAsyncVideoProcessor.h"
#include "AUI/Audio/IAudioPlayer.h"
#include "WebmParser.h"

WebmFramesFactory::WebmFramesFactory(_<IInputStream> stream) {
    mProcessor = _new<AAsyncVideoProcessor>(_new<WebmParser>(std::move(stream)), nullptr);
    mProcessor->run();
}

AImage WebmFramesFactory::provideImage(const glm::ivec2 &size) {
    if (!mPlaybackStarted.time_since_epoch().count()) {
        mPlaybackStarted = std::chrono::system_clock::now();
        mProcessor->playAudio();
    }

    mLastTimeProvided = std::chrono::system_clock::now();
    mLastTimecode = mFrame->timecode;

    auto image = std::move(mFrame->image);
    mFrame.reset();
    return image;
}

bool WebmFramesFactory::isNewImageAvailable() {
    if (!mFrame) {
        mFrame = mProcessor->nextVideoFrame();
        if (!mFrame) {
            return false;
        }
    }

    using namespace std::chrono;
    return mFrame.value().timecode - mLastTimecode <= duration_cast<milliseconds>(system_clock::now() - mLastTimeProvided).count();
}

glm::ivec2 WebmFramesFactory::getSizeHint() {
    return IImageFactory::getSizeHint();
}
