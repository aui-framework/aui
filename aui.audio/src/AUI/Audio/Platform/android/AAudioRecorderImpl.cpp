#include <oboe/Oboe.h>
#include <cassert>
#include "AUI/Audio/Sound/RawSoundStream.h"

class OboeSoundInput : public oboe::AudioStreamDataCallback  {
public:
    static constexpr AAudioFormat OUTPUT_FORMAT = {
            .bitRate = 176400,
            .channelCount = 2,
            .sampleRate = 44100,
            .bitsPerSample = 16
    };

    static OboeSoundInput& instance() {
        static OboeSoundInput p;
        return p;
    }

    void startRecording() {
        mRecordedRawData.clear();
        mRecordedRawData.reserve(0x1000 * 3);
        mStream->requestStart();
    }

    _<RawSoundStream> stopRecording() {
        mStream->requestStop();
        return _new<RawSoundStream>(OUTPUT_FORMAT, std::move(mRecordedRawData));
    }

private:
    OboeSoundInput() {
        oboe::AudioStreamBuilder builder;
        builder.setPerformanceMode(oboe::PerformanceMode::LowLatency)
                ->setSharingMode(oboe::SharingMode::Exclusive)
                ->setDataCallback(this)
                ->setChannelCount(2)
                ->setSampleRate(44100)
                ->setFormat(oboe::AudioFormat::I16)
                ->setDirection(oboe::Direction::Input);
        auto r = builder.openStream(mStream);
        assert(r == oboe::Result::OK);
    }

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override {
        size_t bytesToRead = numFrames * 2 * 2;
        mRecordedRawData.write(reinterpret_cast<char*>(audioData), bytesToRead);
    }

    _<oboe::AudioStream> mStream;
    AByteBuffer mRecordedRawData;
};

void AAudioRecorder::startImpl() {
    OboeSoundInput::instance().startRecording();
}

_<RawSoundStream> AAudioRecorder::stopImpl() {
    return OboeSoundInput::instamce().stopRecording();
}
