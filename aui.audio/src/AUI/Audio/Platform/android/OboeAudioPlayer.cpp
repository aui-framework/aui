#include "OboeAudioPlayer.h"
#include "AUI/Audio/AAudioMixer.h"
#include <oboe/Oboe.h>

class OboeSoundOutput : public oboe::AudioStreamDataCallback {
public:
    static OboeSoundOutput& instance() {
        static OboeSoundOutput p;
        return p;
    }

    [[nodiscard]]
    AAudioMixer& mixer() noexcept {
        return mMixer;
    }

    [[nodiscard]]
    const _<oboe::AudioStream>& stream() const noexcept {
        return mStream;
    }

    [[nodiscard]]
    const _<AAbstractThread>& thread() const noexcept {
        return mThread;
    }

private:
    OboeSoundOutput() {
        oboe::AudioStreamBuilder builder;
        builder.setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setDataCallback(this)
            ->setChannelCount(2)
            ->setSampleRate(44100)
            ->setFormat(oboe::AudioFormat::I16);
        auto r = builder.openStream(mStream);
        AUI_ASSERT(r == oboe::Result::OK);
        mStream->requestStart();
    }

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream* audioStream, void* audioData, int32_t numFrames) override {
        if (mThread == nullptr) {
            mThread = AThread::current();
        }
        AThread::processMessages();
        size_t bytesToRead = numFrames * 2 * 2;
        size_t r = mMixer.readSoundData({ (std::byte*) audioData, bytesToRead });
        if (r < bytesToRead) {
            std::memset((char*) audioData + r, 0, bytesToRead - r);
        }
        return oboe::DataCallbackResult::Continue;
    }

    _<oboe::AudioStream> mStream;
    AAudioMixer mMixer;
    _<AAbstractThread> mThread;
};

void OboeAudioPlayer::playImpl() {
    while (OboeSoundOutput::instance().thread() == nullptr)
        ;
    OboeSoundOutput::instance().thread()->enqueue([self = aui::ptr::shared_from_this(this)]() mutable {
        self->initializeIfNeeded();
        OboeSoundOutput::instance().mixer().addSoundSource(std::move(self));
    });
}

void OboeAudioPlayer::pauseImpl() {
    while (OboeSoundOutput::instance().thread() == nullptr)
        ;
    OboeSoundOutput::instance().thread()->enqueue([self = aui::ptr::shared_from_this(this)]() mutable {
        OboeSoundOutput::instance().mixer().removeSoundSource(self);
    });
}

void OboeAudioPlayer::stopImpl() {
    while (OboeSoundOutput::instance().thread() == nullptr)
        ;
    OboeSoundOutput::instance().thread()->enqueue([self = aui::ptr::shared_from_this(this)]() mutable {
        OboeSoundOutput::instance().mixer().removeSoundSource(self);
        self->reset();
    });
}

void OboeAudioPlayer::onLoopSet() {}

void OboeAudioPlayer::onVolumeSet() {}

namespace aui::oboe {
void onResume() { OboeSoundOutput::instance().stream()->requestStart(); }

void onPause() { OboeSoundOutput::instance().stream()->requestPause(); }
}   // namespace aui::oboe