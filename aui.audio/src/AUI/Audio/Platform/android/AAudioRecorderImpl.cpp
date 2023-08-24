#include "AUI/Audio/AAudioRecorder.h"
#include "AUI/Audio/Sound/ARawSoundStream.h"
#include "AUI/Platform/AMessageBox.h"
#include "AUI/Platform/AWindow.h"
#include <oboe/Oboe.h>
#include <cassert>

class OboeSoundInput : public oboe::AudioStreamDataCallback  {
public:
    static constexpr AAudioFormat OUTPUT_FORMAT = {
            .bitRate = 88200,
            .channelCount = 1,
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

    _<ARawSoundStream> stopRecording() {
        mStream->requestStop();
        return _new<ARawSoundStream>(OUTPUT_FORMAT, std::move(mRecordedRawData));
    }

private:
    OboeSoundInput() {
        oboe::AudioStreamBuilder builder;
        builder.setDirection(oboe::Direction::Input)
                ->setDataCallback(this)
                ->setChannelCount(1)
                ->setSampleRate(44100)
                ->setFormat(oboe::AudioFormat::I16);
        auto r = builder.openStream(mStream);
        assert(r == oboe::Result::OK);
    }

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override {
        size_t bytesToRead = numFrames * 2 * 2;
        mRecordedRawData.write(reinterpret_cast<char*>(audioData), bytesToRead);
        return oboe::DataCallbackResult::Continue;
    }

    _<oboe::AudioStream> mStream;
    AByteBuffer mRecordedRawData;
};

void AAudioRecorder::startImpl() {
    OboeSoundInput::instance().startRecording();
}

_<ARawSoundStream> AAudioRecorder::stopImpl() {
    return OboeSoundInput::instance().stopRecording();
}
