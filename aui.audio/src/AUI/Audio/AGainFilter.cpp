#include "AGainFilter.h"

#include <cmath>

void AGainFilter::process(float* samples, size_t frames, size_t channels, Layout layout) {
    const float target = mTargetGain.load(std::memory_order_relaxed);
    if (!mPrimed) {
        mPrimed = true;
        mCurrentGain = mRampTarget = target; // nothing to smooth from
        mRampLeft = 0;
    }
    if (target != mRampTarget) {
        mRampTarget = target;
        const auto rampFrames = size_t(
            double(mSmoothingMs.load(std::memory_order_relaxed)) * double(mSampleRate.load(std::memory_order_relaxed)) /
            1000.0);
        if (rampFrames == 0) {
            mCurrentGain = target;
            mRampLeft = 0;
        } else {
            mRampLeft = rampFrames;
            mRampStep = (target - mCurrentGain) / float(rampFrames);
        }
    }

    if (mRampLeft == 0 && mCurrentGain == 1.0f) return;

    // advances the ramp by one frame, returns the gain of that frame
    auto advance = [&](float& gain, size_t& left) {
        if (left == 0) return gain;
        gain += mRampStep;
        if (--left == 0) gain = mRampTarget;
        return gain;
    };

    switch (layout) {
        case Layout::INTERLEAVED: {
            float gain = mCurrentGain;
            size_t left = mRampLeft;
            for (size_t f = 0; f < frames; ++f) {
                const float g = advance(gain, left);
                for (size_t c = 0; c < channels; ++c) samples[f * channels + c] *= g;
            }
            mCurrentGain = gain;
            mRampLeft = left;
            break;
        }
        case Layout::PLANAR: {
            float gain = mCurrentGain;
            size_t left = mRampLeft;
            for (size_t c = 0; c < channels; ++c) {
                gain = mCurrentGain;
                left = mRampLeft;
                float* channel = samples + c * frames;
                for (size_t f = 0; f < frames; ++f) channel[f] *= advance(gain, left);
            }
            mCurrentGain = gain;
            mRampLeft = left;
            break;
        }
    }
}

void AGainFilter::setVolume(aui::audio::VolumeLevel volume) noexcept {
    constexpr float DB_RATIO = 60.0f;
    mVolume = volume;
    float normalizedVolume = volume / static_cast<float>(aui::audio::VolumeLevel::MAX);
    float gain = (std::exp(std::log(DB_RATIO) * normalizedVolume) - 1.0f) / (DB_RATIO - 1.0f);
    mTargetGain.store(std::max(std::min(gain, 1.0f), 0.0f), std::memory_order_relaxed);
}

void AGainFilter::setSmoothing(std::chrono::milliseconds smoothing) noexcept {
    mSmoothingMs.store(smoothing.count(), std::memory_order_relaxed);
}

void AGainFilter::setSampleRate(size_t sampleRate) noexcept {
    mSampleRate.store(sampleRate, std::memory_order_relaxed);
}
