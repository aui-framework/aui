#pragma once

#include "AUI/Audio/Sound/ISoundInputStream.h"
#include "AUI/Audio/Mixer/details/ACompileTimeSoundResampler.h"

class AAudioPlayer;

/**
 * @brief Implements audio mixing and resampling.
 * @ingroup audio
 */
class API_AUI_AUDIO ASoundResampler : public ISoundInputStream {
public:
    ASoundResampler(_<ISoundInputStream> stream,
                    ASampleFormat destinationFormat = aui::audio::DEFAULT_OUTPUT_FORMAT) noexcept:
                    mSoundStream(std::move(stream)),
                    mDestinationFormat(destinationFormat) {
        mFormat = mSoundStream->info();
    }

    size_t read(char* dst, size_t size) override;

    AAudioFormat info() override;

    void rewind() override;

private:
    _<ISoundInputStream> mSoundStream;
    ASampleFormat mDestinationFormat;
    AAudioFormat mFormat;
};