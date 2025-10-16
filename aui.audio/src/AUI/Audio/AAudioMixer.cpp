#include "AAudioMixer.h"
#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Common/ASmallVector.h"
#include "AUI/Logging/ALogger.h"
#include "Platform/RequestedAudioFormat.h"
#include "Util.h"

void AAudioMixer::addSoundSource(aui::non_null<_<IAudioPlayer>> s) {
#if AUI_DEBUG
    std::unique_lock lock(mConcurrentAccessCheck, std::try_to_lock);
    AUI_ASSERTX(lock.owns_lock(), "concurrent access to AAudioMixer is not allowed");
#endif
    mPlayers.push_back(std::move(s.value));
}

void AAudioMixer::removeSoundSource(aui::non_null<_<IAudioPlayer>> s) {
#if AUI_DEBUG
    std::unique_lock lock(mConcurrentAccessCheck, std::try_to_lock);
    AUI_ASSERTX(lock.owns_lock(), "concurrent access to AAudioMixer is not allowed");
#endif
    mPlayers.erase(std::remove(mPlayers.begin(), mPlayers.end(), s.value), mPlayers.end());
}

size_t AAudioMixer::readSoundData(std::span<std::byte> destination) {
    std::memset(destination.data(), 0, destination.size());
    size_t samples_requested = destination.size() / aui::audio::bytesPerSample(aui::audio::platform::requested_sample_format);
    ASmallVector<_<IAudioPlayer>, 8> itemsToRemove;
    size_t result = 0;
    aui::impl::reserveVector(mMixBuffer, samples_requested);
    std::memset(mMixBuffer.data(), 0, samples_requested * sizeof(float));
    {
        std::unique_lock lock(mConcurrentAccessCheck, std::try_to_lock);
        AUI_ASSERTX(lock.owns_lock(), "concurrent access to AAudioMixer is not allowed");

        mPlayers.erase(
            std::remove_if(
                mPlayers.begin(), mPlayers.end(),
                [&](_<IAudioPlayer>& player) {
                    auto url = [&] {
                      return player->url().map(&AUrl::full).valueOr("unknown url");
                    };
                    try {
                        if (!player->resamplerStream()) {
                            // no player stream = no player.
                            // apparently, the player should have been removed already.
                            // but just in case (i.e., weird race condition), we'll remove it anyway.
                            ALogger::warn("AAudioMixer") << url() << " : internal error: player stream is null, removing player";
                            itemsToRemove << std::move(player);
                            return true;
                        }
                        aui::impl::reserveVector(mReadBuffer, samples_requested);
                        std::memset(mReadBuffer.data(), 0, samples_requested * sizeof(float));
                        size_t r = player->resamplerStream()->read({reinterpret_cast<std::byte*>(mReadBuffer.data()), samples_requested * sizeof(float)});
                        AUI_EMIT_FOREIGN(player, read);
                        if (r == 0) {
                            if (player->loop()) {
                                player->rewind();
                                return false;
                            }
                            itemsToRemove << std::move(player);
                            return true;   // remove item
                        }
                        size_t samplesRead = r / sizeof(float);
                        for (size_t i = 0; i < samplesRead; ++i) {
                            mMixBuffer[i] += mReadBuffer[i];
                        }
                        result = std::max(r, result);
                        return false;
                    } catch (const AException& e) {
                        ALogger::err("AAudioMixer") << "An error occurred during audio playback of (" << url() << "), the broken player will be removed: " << e;
                    } catch (const std::exception& e) {
                        ALogger::err("AAudioMixer") << "An error occurred during audio playback of (" << url() << "), the broken player will be removed: " << e.what();
                    } catch (...) {
                        ALogger::err("AAudioMixer") << "An error occurred during audio playback of ( " << url() << "), the broken player will be removed";
                    }

                    // if an error occured during playback, remove player from player list
                    itemsToRemove << std::move(player);
                    return true;
                }),
            mPlayers.end());
    }
    for (auto& player : itemsToRemove) {
        player->onFinished();
    }
    for (size_t i = 0; i < samples_requested; ++i) {
        mMixBuffer[i] = std::clamp(mMixBuffer[i], -1.0f, 1.0f);
    }
    aui::audio::convertSampleFormat(ASampleFormat::F32, aui::audio::platform::requested_sample_format, reinterpret_cast<const char*>(mMixBuffer.data()), reinterpret_cast<char*>(destination.data()), samples_requested);

    return (result / sizeof(float)) * aui::audio::bytesPerSample(aui::audio::platform::requested_sample_format);
}

AAudioMixer::~AAudioMixer() {
    std::unique_lock lock(mConcurrentAccessCheck);
    mPlayers.clear();
}
