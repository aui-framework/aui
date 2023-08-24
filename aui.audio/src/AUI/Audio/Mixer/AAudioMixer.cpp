#include "AAudioMixer.h"
#include "AUI/Audio/Mixer/ASampleCommitter.h"
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

size_t AAudioMixer::requestSoundData(char* dst, size_t size) {
    std::memset(dst, 0, size);
    std::unique_lock lock(mMutex);
    std::list<_<ISoundSource>> itemsToRemove;
    size_t result = 0;
    for (auto& source : mSoundSources) {
        size_t r = source->requestSoundData(dst, size);
        if (r == 0) {
            if (!source->getConfig().loop || !source->requestRewind()) {
                itemsToRemove.push_back(source);
            }
        }
        else {
            result = std::max(r, result);
        }
    }
    for (const auto& i : itemsToRemove) {
        mSoundSources.erase(std::remove(mSoundSources.begin(), mSoundSources.end(), i),
                            mSoundSources.end());
    }
    return result;
}
