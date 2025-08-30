#include "AGainFilter.h"

#include <cmath>

void AGainFilter::process(float* samples, size_t num_samples) {
    for (size_t i = 0; i < num_samples; i++) {
        samples[i] *= mGain;
    }
}

void AGainFilter::setVolume(aui::audio::VolumeLevel volume) noexcept {
    constexpr float DB_RATIO = 60.0f;
    mVolume = volume;
    float normalizedVolume = volume / static_cast<float>(aui::audio::VolumeLevel::MAX);
    mGain = (std::exp(std::log(DB_RATIO) * normalizedVolume) - 1.0f) / (DB_RATIO - 1.0f);
    mGain = std::max(std::min(mGain, 1.0f), 0.0f);
}
