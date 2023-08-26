#include "AUI/Audio/AAudioPlayer.h"
#include "AUI/Audio/Mixer/AAudioMixer.h"
#include "AUI/Audio/Mixer/ASampleCommitter.h"
#include <oboe/Oboe.h>

class OboeSoundOutput : public oboe::AudioStreamDataCallback  {
public:
    static OboeSoundOutput& instance() {
        static OboeSoundOutput p;
        return p;
    }

    void addSource(_<ISoundSource> source) {
        mMixer->addSoundSource(std::move(source));
    }

    void removeSource(const _<ISoundSource>& source) {
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
    assert(mCommitter == nullptr);
    mCommitter = _new<ASampleCommitter>(mSource, PlaybackConfig{.loop = mLoop, .volume = mVolume});
    OboeSoundOutput::instance().addSource(mCommitter);
}

void AAudioPlayer::pauseImpl() {
    OboeSoundOutput::instance().removeSource(mCommitter);
    mCommitter.reset();
}

void AAudioPlayer::stopImpl() {
    OboeSoundOutput::instance().removeSource(mCommitter);
    mSource->rewind();
    mCommitter.reset();
}

void AAudioPlayer::setSourceImpl() {
    //mCommitter = _new<ASampleCommitter>(mSource, PlaybackConfig{.loop = mLoop, .volume = mVolume});
}
