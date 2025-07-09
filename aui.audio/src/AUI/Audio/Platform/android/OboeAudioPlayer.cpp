#include "OboeAudioPlayer.h"
#include "AUI/Audio/AAudioMixer.h"
#include "AUI/Audio/ASoundResampler.h"
#include <oboe/Oboe.h>

class OboeSoundOutput : public oboe::AudioStreamDataCallback  {
public:
    static OboeSoundOutput& instance() {
        static OboeSoundOutput p;
        return p;
    }

    void addSource(_<OboeAudioPlayer> source) {
        mMixer->addSoundSource(std::move(source));
    }

    void removeSource(const _<OboeAudioPlayer>& source) {
        mMixer->removeSoundSource(source);
    }

    [[nodiscard]]
    const _<oboe::AudioStream>& stream() const noexcept {
        return mStream;
    }

private:
    OboeSoundOutput() : mMixer(_new<AAudioMixer>()) {
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

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override {
        size_t bytesToRead = numFrames * 2 * 2;
        size_t r = mMixer->readSoundData({(std::byte*)audioData, bytesToRead});
        if (r < bytesToRead) {
            std::memset((char*)audioData + r, 0, bytesToRead - r);
        }
        return oboe::DataCallbackResult::Continue;
    }

    _<oboe::AudioStream> mStream;
    _<AAudioMixer> mMixer;
};

void OboeAudioPlayer::playImpl() {
    initializeIfNeeded();
    OboeSoundOutput::instance().addSource(_cast<OboeAudioPlayer>(aui::ptr::shared_from_this(this)));
}

void OboeAudioPlayer::pauseImpl() {
    OboeSoundOutput::instance().removeSource(_cast<OboeAudioPlayer>(aui::ptr::shared_from_this(this)));
}

void OboeAudioPlayer::stopImpl() {
    OboeSoundOutput::instance().removeSource(_cast<OboeAudioPlayer>(aui::ptr::shared_from_this(this)));
    reset();
}

void OboeAudioPlayer::onLoopSet() {

}

void OboeAudioPlayer::onVolumeSet() {

}

namespace aui::oboe {
    void onResume() {
        OboeSoundOutput::instance().stream()->requestStart();
    }

    void onPause() {
        OboeSoundOutput::instance().stream()->requestPause();
    }
}