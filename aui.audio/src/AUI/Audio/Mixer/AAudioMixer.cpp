#include "AAudioMixer.h"
#include "AUI/Audio/Mixer/ASoundResampler.h"
#include "AUI/Audio/Mixer/ISoundSource.h"

void AAudioMixer::addSoundSource(_<ISoundSource> s) {
    std::unique_lock lock(mMutex);
    mSoundSources.push_back(std::move(s));
}

void AAudioMixer::removeSoundSource(const _<ISoundSource>& s) {
    std::unique_lock lock(mMutex);
    mSoundSources.erase(std::remove(mSoundSources.begin(),
                                    mSoundSources.end(),
                                    s), mSoundSources.end());
}

size_t AAudioMixer::readSoundData(std::span<std::byte> destination) {
    std::memset(destination.data(), 0, destination.size());
    std::unique_lock lock(mMutex);

    size_t result = 0;
    mSoundSources.erase(std::remove_if(mSoundSources.begin(), mSoundSources.end(), [&](const _<ISoundSource>& source) {
        size_t r = source->readSoundData(destination);
        if (r == 0) {
            if (!source->getConfig().loop || !source->requestRewind()) {
                return true; // remove item
            }
        }
        else {
            result = std::max(r, result);
        }
        return false;
    }), mSoundSources.end());
    return result;
}
