#include "APlayerSoundStream.h"
#include "AUI/Audio/AAudioPlayer.h"
#include "ACompileTimeSoundResampler.h"
#include "AUI/Audio/AAudioFormat.h"
#include "AUI/Audio/ASampleFormat.h"

APlayerSoundStream::APlayerSoundStream(AAudioPlayer* player) : mPlayer(player) {
    assert(player != nullptr);
}

size_t APlayerSoundStream::read(char* dst, size_t size) {
    size_t bytesRead = 0;
    std::byte buffer[BUFFER_SIZE];
    auto sampleFormat = mPlayer->source()->info().sampleFormat;
    auto bytesPerSample = mPlayer->source()->info().bitsPerSample() / 8;
    while (bytesRead < size) {
        auto currentRead = mPlayer->source()->read(std::span(buffer, std::min(size, sizeof(buffer))));
        if (currentRead == 0) {
            break;
        }

        VolumeLevel volumeLevel = static_cast<int32_t>(256 * mPlayer->volume());
        for (std::byte* it = buffer; it < buffer + currentRead; it += bytesPerSample) {
            switch (sampleFormat) {
                case ASampleFormat::I16: {
                    processSample<ASampleFormat::I16>(it, reinterpret_cast<std::byte*&>(dst), volumeLevel);
                    break;
                }

                case ASampleFormat::I24: {
                    processSample<ASampleFormat::I24>(it, reinterpret_cast<std::byte*&>(dst), volumeLevel);
                    break;
                }
            }
        }

        bytesRead += currentRead;
    }

    return bytesRead;
}

AAudioFormat APlayerSoundStream::info() {
    return mPlayer->source()->info();
}

void APlayerSoundStream::rewind() {
    mPlayer->source()->rewind();
}

template<ASampleFormat format>
void APlayerSoundStream::processSample(std::byte *src, std::byte* &dst, VolumeLevel volumeLevel) {
    auto sample = aui::audio::util::extractSample<format>(src);
    aui::audio::util::pushSample<format>((static_cast<int32_t>(sample) * volumeLevel) / MAX_VOLUME_LEVEL, reinterpret_cast<std::byte*>(dst));
    dst += aui::audio::util::size_bytes<format>();
}
