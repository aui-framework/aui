#include "ASoundResampler.h"
#include "AUI/Audio/Mixer/details/ACompileTimeSoundResampler.h"
#include "AUI/Audio/AAudioPlayer.h"

ASoundResampler::ASoundResampler(_<ISoundInputStream> stream, APlaybackConfig config) :
            mSoundStream(std::move(stream)), mConfig(config) {
    mFormat = mSoundStream->info();
}

size_t ASoundResampler::readSoundData(std::span<std::byte> destination) {
    switch (mFormat.bitsPerSample) {
        case 16: {
            ACompileTimeSoundResampler<ASampleFormat::I16, aui::audio::DEFAULT_OUTPUT_FORMAT> resampler(destination);
            resampler.commitAllSamples(mSoundStream);
            return resampler.writtenSize();
        }
        case 24: {
            ACompileTimeSoundResampler<ASampleFormat::I24, aui::audio::DEFAULT_OUTPUT_FORMAT> resampler(destination);
            resampler.commitAllSamples(mSoundStream);
            return resampler.writtenSize();
        }
    }
    throw AException("invalid mFormat.bitsPerSample = {}"_format(mFormat.bitsPerSample));
}

bool ASoundResampler::requestRewind() {
    mSoundStream->rewind();
    return true;
}

APlaybackConfig ASoundResampler::getConfig() {
    return mConfig;
}
