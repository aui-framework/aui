#include "WebmFramesFactory.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Video/AAsyncVideoProcessor.h"
#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/IO/APortionedMultipleStream.h"
#include "WebmParser.h"

#include "AUI/Audio/Formats/opus/AOpusSoundStream.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"

namespace {
    _<ISoundInputStream> makeSoundSource(_<IInputStream> source, aui::audio::Codec codec) {
        switch (codec) {
            case aui::audio::Codec::VORBIS:
                return _new<AOpusSoundStream>(std::move(source));
            case aui::audio::Codec::OPUS:
                return _new<AOpusSoundStream>(std::move(source));
            default:
                return nullptr;
        }
    }
}

WebmFramesFactory::WebmFramesFactory(_<IInputStream> stream) {
    mProcessor = _new<AAsyncVideoProcessor>(_new<WebmParser>(std::move(stream)), nullptr);
    mProcessor->run();
    mAudioSource = _new<APortionedMultipleStream>();
}

AImage WebmFramesFactory::provideImage(const glm::ivec2 &size) {
    if (!mPlaybackStarted.time_since_epoch().count()) {
        mPlaybackStarted = std::chrono::system_clock::now();
        initialize();
    }

    while (auto audioFrame = mProcessor->nextAudioFrame()) {
        mAudioSource << _new<AStrongByteBufferInputStream>(std::move(audioFrame->frameData));
        if (audioFrame.value().timecode >= mLastTimecode) {
            break;
        }
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

void WebmFramesFactory::initialize() {
    if (auto audioInfo = mProcessor->audioInfo()) {
        mAudioSource << _new<AStrongByteBufferInputStream>(std::move(audioInfo->header));
        mPlayer = IAudioPlayer::fromSoundStream(makeSoundSource(mAudioSource, audioInfo->codec));
        mPlayer->setLoop(true);
        mPlayer->play();
    }
}
