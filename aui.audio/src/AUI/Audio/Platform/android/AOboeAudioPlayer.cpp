#include "AOboeAudioPlayer.h"
#include "AUI/Audio/AAudioMixer.h"
#include "AUI/Audio/ASoundResampler.h"
#include <oboe/Oboe.h>

_<IAudioPlayer> IAudioPlayer::fromSoundStream(_<ISoundInputStream> stream) {
    auto result = _new<AOboeAudioPlayer>();
    result->setSource(std::move(stream));
    return result;
}

class OboeSoundOutput : public oboe::AudioStreamDataCallback  {
public:
    static OboeSoundOutput& instance() {
        static OboeSoundOutput p;
        return p;
    }

    void addSource(_<AOboeAudioPlayer> source) {
        mMixer->addSoundSource(std::move(source));
    }

    void removeSource(const _<AOboeAudioPlayer>& source) {
        mMixer->removeSoundSource(source);
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
        assert(r == oboe::Result::OK);
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

void AOboeAudioPlayer::playImpl() {
    assert(mResampled == nullptr);
    mResampled = _new<ASoundResampler>(this, aui::audio::DEFAULT_OUTPUT_FORMAT);
    OboeSoundOutput::instance().addSource(_cast<AOboeAudioPlayer>(sharedPtr()));
}

void AOboeAudioPlayer::pauseImpl() {
    OboeSoundOutput::instance().removeSource(_cast<AOboeAudioPlayer>(sharedPtr()));
    mResampled.reset();
}

void AOboeAudioPlayer::stopImpl() {
    OboeSoundOutput::instance().removeSource(_cast<AOboeAudioPlayer>(sharedPtr()));
    source()->rewind();
    mResampled.reset();
}

void AOboeAudioPlayer::onSourceSet() {

}

void AOboeAudioPlayer::onLoopSet() {

}

void AOboeAudioPlayer::onVolumeSet() {

}
