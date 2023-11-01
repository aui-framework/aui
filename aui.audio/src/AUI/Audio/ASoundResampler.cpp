#include "ASoundResampler.h"
#include "AUI/Audio/ACompileTimeSoundResampler.h"
#include "AUI/Audio/IAudioPlayer.h"

ASoundResampler::ASoundResampler(const _<IAudioPlayer> &player) noexcept {
    mParentPlayer = player;
    mSoundStream = player->source();
    mInputFormat = mSoundStream->info();
}

size_t ASoundResampler::read(char* dst, size_t size) {
    std::span<std::byte> destination(reinterpret_cast<std::byte*>(dst), size);
    // todo some sort of helper in like: for_each_value<I16, I24>([]<ASampleFormat v>() {
    //  })
    switch (mInputFormat.sampleFormat) {
        case ASampleFormat::I16: {
            switch (mInputFormat.channelCount) {
                case AChannelFormat::MONO:
                    return commitSamples<ASampleFormat::I16, AChannelFormat::MONO>(destination);
                case AChannelFormat::STEREO:
                    return commitSamples<ASampleFormat::I16, AChannelFormat::STEREO>(destination);
            }
            throw AException("invalid input channel count = {}"_format(int(mInputFormat.channelCount)));
        }

        case ASampleFormat::I24: {
            switch (mInputFormat.channelCount) {
                case AChannelFormat::MONO:
                    return commitSamples<ASampleFormat::I24, AChannelFormat::MONO>(destination);
                case AChannelFormat::STEREO:
                    return commitSamples<ASampleFormat::I24, AChannelFormat::STEREO>(destination);
            }
            throw AException("invalid input channel format = {}"_format(int(mInputFormat.channelCount)));
        }
    }

    throw AException("invalid input sample format = {}"_format(int(mInputFormat.sampleFormat)));
}

AAudioFormat ASoundResampler::info() {
    return DEFAULT_OUTPUT_FORMAT;
}

void ASoundResampler::rewind() {
    mSoundStream->rewind();
}

template<ASampleFormat inputSampleFormat, AChannelFormat inputChannelsFormat>
size_t ASoundResampler::commitSamples(std::span<std::byte> dst) {
    ACompileTimeSoundResampler<inputSampleFormat, inputChannelsFormat> resampler(dst);
    if (auto player = mParentPlayer.lock(); player && player->volume() != IAudioPlayer::VolumeLevel::MAX) {
        resampler.setVolume(player->volume());
    }

    resampler.commitAllSamples(mSoundStream);
    return resampler.writtenSize();
}
