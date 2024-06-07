#include "AAudioMixer.h"
#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Common/ASmallVector.h"
#include "AUI/Logging/ALogger.h"

void AAudioMixer::addSoundSource(_<IAudioPlayer> s) {
    std::unique_lock lock(mMutex);
    mPlayers.push_back(std::move(s));
}

void AAudioMixer::removeSoundSource(const _<IAudioPlayer>& s) {
    std::unique_lock lock(mMutex);
    mPlayers.erase(std::remove(mPlayers.begin(),
                               mPlayers.end(),
                               s), mPlayers.end());
}

size_t AAudioMixer::readSoundData(std::span<std::byte> destination) {
    std::memset(destination.data(), 0, destination.size());
    ASmallVector<_<IAudioPlayer>, 8> itemsToRemove;
    size_t result = 0;
    {
        std::unique_lock lock(mMutex);

        mPlayers.erase(std::remove_if(mPlayers.begin(), mPlayers.end(), [&](_<IAudioPlayer>& player) {
            try {
                size_t r = player->resampledStream()->read(destination);
                if (r == 0) {
                    if (player->loop()) {
                        player->rewind();
                        return false;
                    }
                    itemsToRemove << std::move(player);
                    return true; // remove item
                }
                result = std::max(r, result);
                return false;
            }
            catch (const AException& e) {
                ALogger::err("audio") << "An error occured during audio playback: " << e;
            }
            catch (const std::exception& e) {
                ALogger::err("audio") << "An error occured during audio playback: " << e.what();
            }
            catch (...) {
                ALogger::err("audio") << "An error occured during audio playback";
            }

            //if an error occured during playback, remove player from player list
            return true;
        }), mPlayers.end());
    }
    for (const auto& player: itemsToRemove) {
        player->onFinished();
    }

    return result;
}
