#pragma once

#include "AUI/Audio/Mixer/details/ISoundSource.h"
#include "AUI/Audio/Sound/ISoundStream.h"
#include "AUI/Audio/Mixer/details/ASampleConsumer.h"

class ASampleCommitter : public ISoundSource {
public:
    explicit ASampleCommitter(_<ISoundStream> stream) : mSoundStream(std::move(stream)) {
        mFormat = mSoundStream->info();
    }

    size_t requestSoundData(char* dst, size_t size) override {
        ASampleConsumer consumer(dst, dst + size);
        switch (mFormat.bitsPerSample) {
            case 16:
                consumer.commitAllSamples<SampleFormat::I16>(mSoundStream);
                break;
            case 24:
                consumer.commitAllSamples<SampleFormat::I24>(mSoundStream);
                break;
        }
        return consumer.writtenSize();
    }

private:
    _<ISoundStream> mSoundStream;
    AAudioFormat mFormat;
};