#include "ASoundResampler.h"
#include "AUI/Audio/ACompileTimeSoundResampler.h"
#include "AUI/Audio/IAudioPlayer.h"

ASoundResampler::ASoundResampler(IAudioPlayer *player, ASampleFormat destinationFormat) noexcept:
        mParentPlayer(player), mDestinationFormat(destinationFormat) {
    assert(player != nullptr);
    mSoundStream = player->source();
    assert(mSoundStream != nullptr);
    mFormat = mSoundStream->info();
}

size_t ASoundResampler::read(char* dst, size_t size) {
    std::span<std::byte> destination((std::byte*)dst, size);
    // todo some sort of helper in like: for_each_value<I16, I24>([]<ASampleFormat v>() {
    //  })
    switch (mFormat.sampleFormat) {
        case ASampleFormat::I16: {
            switch (mDestinationFormat) {
                case ASampleFormat::I16:
                    return commitSamples<ASampleFormat::I16, ASampleFormat::I16>(destination);
                case ASampleFormat::I24:
                    return commitSamples<ASampleFormat::I16, ASampleFormat::I24>(destination);
            }
            throw AException("invalid mDestinationFormat = {}"_format((int)mDestinationFormat));
        }
        case ASampleFormat::I24: {
            switch (mDestinationFormat) {
                case ASampleFormat::I16:
                    return commitSamples<ASampleFormat::I24, ASampleFormat::I16>(destination);
                case ASampleFormat::I24:
                    return commitSamples<ASampleFormat::I24, ASampleFormat::I24>(destination);
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

template<ASampleFormat in, ASampleFormat out>
size_t ASoundResampler::commitSamples(std::span<std::byte> dst) {
    ACompileTimeSoundResampler<in, out> resampler(dst);
    if (mParentPlayer->volume() != IAudioPlayer::VolumeLevel::MAX) {
        resampler.setVolume(mParentPlayer->volume());
    }

    resampler.commitAllSamples(mSoundStream);
    return resampler.writtenSize();
}
