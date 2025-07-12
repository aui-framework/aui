#include "AAudioMixer.h"
#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Common/ASmallVector.h"
#include "AUI/Logging/ALogger.h"

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
    ASmallVector<_<IAudioPlayer>, 8> itemsToRemove;
    size_t result = 0;
    {
        std::unique_lock lock(mConcurrentAccessCheck, std::try_to_lock);
        AUI_ASSERTX(lock.owns_lock(), "concurrent access to AAudioMixer is not allowed");

        mPlayers.erase(
            std::remove_if(
                mPlayers.begin(), mPlayers.end(),
                [&](_<IAudioPlayer>& player) {
                    try {
                        size_t r = player->resampledStream()->read(destination);
                        AUI_EMIT_FOREIGN(player, read);
                        if (r == 0) {
                            if (player->loop()) {
                                player->rewind();
                                return false;
                            }
                            itemsToRemove << std::move(player);
                            return true;   // remove item
                        }
                        result = std::max(r, result);
                        return false;
                    } catch (const AException& e) {
                        ALogger::err("audio") << "An error occurred during audio playback: " << e;
                    } catch (const std::exception& e) {
                        ALogger::err("audio") << "An error occurred during audio playback: " << e.what();
                    } catch (...) {
                        ALogger::err("audio") << "An error occurred during audio playback";
                    }

                    // if an error occured during playback, remove player from player list
                    return true;
                }),
            mPlayers.end());
    }
    for (auto& player : itemsToRemove) {
        player->onFinished();
    }

    return result;
}

AAudioMixer::~AAudioMixer() {
    std::unique_lock lock(mConcurrentAccessCheck);
    mPlayers.clear();
}
