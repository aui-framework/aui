#include "AUI/Audio/AAudioPlayer.h"
#include "AUI/Audio/AAudioMixer.h"
#include "AUI/Audio/ASoundResampler.h"
#include <oboe/Oboe.h>

class OboeSoundOutput : public oboe::AudioStreamDataCallback  {
public:
    static OboeSoundOutput& instance() {
        static OboeSoundOutput p;
        return p;
    }

    void addSource(_<AAudioPlayer> source) {
        mMixer->addSoundSource(std::move(source));
    }

    void removeSource(const _<AAudioPlayer>& source) {
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


AAudioPlayer::AAudioPlayer() {

}

AAudioPlayer::~AAudioPlayer() {
}

AAudioPlayer::AAudioPlayer(_<ISoundInputStream> stream) {
    setSource(std::move(stream));
}

void AAudioPlayer::playImpl() {
    assert(mResampler == nullptr);
    mResampler = _new<ASoundResampler>(mSource);
    OboeSoundOutput::instance().addSource(_cast<AAudioPlayer>(sharedPtr()));
}

void AAudioPlayer::pauseImpl() {
    assert(mResampler != nullptr);
    OboeSoundOutput::instance().removeSource(_cast<AAudioPlayer>(sharedPtr()));
    mResampler.reset();
}

void AAudioPlayer::stopImpl() {
    assert(mResampler != nullptr);
    OboeSoundOutput::instance().removeSource(_cast<AAudioPlayer>(sharedPtr()));
    mSource->rewind();
    mResampler.reset();
}

void AAudioPlayer::onSourceSet() {
    //mResampler = _new<ASoundResampler>(mSource, APlaybackConfig{.loop = mLoop, .volume = mVolume});
}
