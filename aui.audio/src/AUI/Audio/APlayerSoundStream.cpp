#include "APlayerSoundStream.h"
#include "AUI/Audio/AAudioPlayer.h"
#include "ACompileTimeSoundResampler.h"
#include "AUI/Audio/AAudioFormat.h"
#include "AUI/Audio/ASampleFormat.h"

APlayerSoundStream::APlayerSoundStream(_weak<AAudioPlayer> player) : mPlayer(std::move(player)) {

}

size_t APlayerSoundStream::read(char* dst, size_t size) {
    if (auto player = mPlayer.lock()) {
        size_t bytesRead = 0;
        std::byte buffer[BUFFER_SIZE];
        auto sampleFormat = player->source()->info().sampleFormat;
        auto bytesPerSample = player->source()->info().bitsPerSample() / 8;
        while (bytesRead < size) {
            auto currentRead = player->source()->read(std::span(buffer, std::min(size, sizeof(buffer))));
            if (currentRead == 0) {
                break;
            }

            auto volumeLevel = player->volume();
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

    return 0;
}

AAudioFormat APlayerSoundStream::info() {
    if (auto player = mPlayer.lock()) {
        return player->source()->info();
    }
    assert(("player no longer exists", false));
    return AAudioFormat{};
}

void APlayerSoundStream::rewind() {
    if (auto player = mPlayer.lock()) {
        player->source()->rewind();
    }
}

template<ASampleFormat format>
void APlayerSoundStream::processSample(std::byte *src, std::byte* &dst, aui::float_within_0_1 volumeLevel) {
    auto sample = aui::audio::util::extractSample<format>(src);
    sample = static_cast<decltype(sample)>(static_cast<float>(sample) * volumeLevel);
    aui::audio::util::pushSample<format>(sample, reinterpret_cast<std::byte*>(dst));
    dst += aui::audio::util::size_bytes<format>();
}
