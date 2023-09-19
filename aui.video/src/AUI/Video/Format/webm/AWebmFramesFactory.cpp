#include "AWebmFramesFactory.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Video/AAsyncVideoProcessor.h"
#include "AWebmParser.h"

AWebmFramesFactory::AWebmFramesFactory(_<IInputStream> stream) {
    mProcessor = _new<AAsyncVideoProcessor>(_new<AWebmParser>(std::move(stream)), nullptr);
    mProcessor->run();
}

AImage AWebmFramesFactory::provideImage(const glm::ivec2 &size) {
    ARaiiHelper helper = [this]() {
        mFrame.reset();
        if (!mPlaybackStarted.time_since_epoch().count()) {
            mPlaybackStarted = std::chrono::system_clock::now();
        }
    };

    return std::move((*mFrame).image);
}

bool AWebmFramesFactory::isNewImageAvailable() {
    if (!mFrame) {
        mFrame = mProcessor->nextFrame();
        if (!mFrame) {
            return false;
        }
    }

    using namespace std::chrono;
    return mFrame.value().timecode <= duration_cast<milliseconds>(system_clock::now() - mPlaybackStarted).count();
}

glm::ivec2 AWebmFramesFactory::getSizeHint() {
    return IImageFactory::getSizeHint();
}
