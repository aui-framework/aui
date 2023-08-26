#include "ASampleCommitter.h"
#include "AUI/Audio/Mixer/details/ASampleResampler.h"
#include "AUI/Audio/AAudioPlayer.h"

ASampleCommitter::ASampleCommitter(_<ISoundInputStream> stream, PlaybackConfig config) :
            mSoundStream(std::move(stream)), mConfig(config) {
    mFormat = mSoundStream->info();
}

size_t ASampleCommitter::readSoundData(std::span<std::byte> destination) {
    switch (mFormat.bitsPerSample) {
        case 16: {
            ASampleResampler<SampleFormat::I16, aui::audio::DEFAULT_OUTPUT_FORMAT> resampler(destination);
            resampler.commitAllSamples(mSoundStream);
            return resampler.writtenSize();
        }
        case 24: {
            ASampleResampler<SampleFormat::I24, aui::audio::DEFAULT_OUTPUT_FORMAT> resampler(destination);
            resampler.commitAllSamples(mSoundStream);
            return resampler.writtenSize();
        }
    }
    throw AException("invalid mFormat.bitsPerSample = {}"_format(mFormat.bitsPerSample));
}

bool ASampleCommitter::requestRewind() {
    mSoundStream->rewind();
    return true;
}

PlaybackConfig ASampleCommitter::getConfig() {
    return mConfig;
}
