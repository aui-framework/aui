#include "AUI/Audio/AAudioPlayer.h"
#include "AUI/Audio/Mixer/AAudioMixer.h"
#include "AUI/Audio/Mixer/ASoundResampler.h"
#include <oboe/Oboe.h>

class OboeSoundOutput : public oboe::AudioStreamDataCallback  {
public:
    static OboeSoundOutput& instance() {
        static OboeSoundOutput p;
        return p;
    }

    void addSource(_<ISoundInputStream> source) {
        mMixer->addSoundSource(std::move(source));
    }

    void removeSource(const _<ISoundInputStream>& source) {
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
        auto dst = static_cast<char *>(audioData);
        size_t r = mMixer->readSoundData(dst, bytesToRead);
        if (r < bytesToRead) {
            std::memset(dst + r, 0, bytesToRead - r);
        }
        return oboe::DataCallbackResult::Continue;
    }

    _<oboe::AudioStream> mStream;
    _<AAudioMixer> mMixer;
};

void AAudioPlayer::playImpl() {
    assert(mResampler == nullptr);
    mResampler = _new<ASoundResampler>(mSource, APlaybackConfig{.loop = mLoop, .volume = mVolume});
    OboeSoundOutput::instance().addSource(mResampler);
}

void AAudioPlayer::pauseImpl() {
    assert(mResampler != nullptr);
    OboeSoundOutput::instance().removeSource(mResampler);
    mResampler.reset();
}

void AAudioPlayer::stopImpl() {
    assert(mResampler != nullptr);
    OboeSoundOutput::instance().removeSource(mResampler);
    mSource->rewind();
    mResampler.reset();
}

void AAudioPlayer::onSourceSet() {
    //mResampler = _new<ASoundResampler>(mSource, APlaybackConfig{.loop = mLoop, .volume = mVolume});
}
