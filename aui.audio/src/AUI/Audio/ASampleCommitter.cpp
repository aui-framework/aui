#include "ASoundResampler.h"
#include "AUI/Audio/ACompileTimeSoundResampler.h"
#include "AUI/Audio/AAudioPlayer.h"


size_t ASoundResampler::read(char* dst, size_t size) {
    std::span<std::byte> destination((std::byte*)dst, size);
    // todo some sort of helper in like: for_each_value<I16, I24>([]<ASampleFormat v>() {
    //  })
    switch (mFormat.sampleFormat) {
        case ASampleFormat::I16: {
            switch (mDestinationFormat) {
                case ASampleFormat::I16: {
                    ACompileTimeSoundResampler<ASampleFormat::I16, ASampleFormat::I16> resampler(destination);
                    resampler.commitAllSamples(mSoundStream);
                    return resampler.writtenSize();
                }
                case ASampleFormat::I24: {
                    ACompileTimeSoundResampler<ASampleFormat::I16, ASampleFormat::I24> resampler(destination);
                    resampler.commitAllSamples(mSoundStream);
                    return resampler.writtenSize();
                }
            }
            throw AException("invalid mDestinationFormat = {}"_format((int)mDestinationFormat));
        }
        case ASampleFormat::I24: {
            switch (mDestinationFormat) {
                case ASampleFormat::I16: {
                    ACompileTimeSoundResampler<ASampleFormat::I24, ASampleFormat::I16> resampler(destination);
                    resampler.commitAllSamples(mSoundStream);
                    return resampler.writtenSize();
                }
                case ASampleFormat::I24: {
                    ACompileTimeSoundResampler<ASampleFormat::I24, ASampleFormat::I24> resampler(destination);
                    resampler.commitAllSamples(mSoundStream);
                    return resampler.writtenSize();
                }
            }
            throw AException("invalid mDestinationFormat = {}"_format((int)mDestinationFormat));
        }
    }
    throw AException("invalid mFormat.format = {}"_format((int)mFormat.sampleFormat));
}

AAudioFormat ASoundResampler::info() {
    auto i = mSoundStream->info();
    i.sampleFormat = mDestinationFormat;
    return i;
}

void ASoundResampler::rewind() {
    mSoundStream->rewind();
}

