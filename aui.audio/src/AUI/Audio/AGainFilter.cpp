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
    //mGain = (std::exp(std::log(DB_RATIO) * (volume / static_cast<float>(aui::audio::VolumeLevel::MAX))) - 1.0f) / (DB_RATIO - 1.0f);
}
