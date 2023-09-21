#include "AAudioMixer.h"
#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Audio/ISoundInputStream.h"

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
    std::unique_lock lock(mMutex);

    size_t result = 0;
    mPlayers.erase(std::remove_if(mPlayers.begin(), mPlayers.end(), [&](const _<IAudioPlayer>& player) {
        size_t r = player->resampledStream()->read(destination);
        if (r == 0) {
            if (player->loop()) {
                player->resampledStream()->rewind();
                return false;
            }
            AUI_EMIT_FOREIGN(player, finished);
            return true; // remove item
        }
        else {
            result = std::max(r, result);
        }
        return false;
    }), mPlayers.end());
    return result;
}
