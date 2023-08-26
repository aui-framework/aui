#pragma once

/**
 * @brief Info about playback.
 * @ingroup audio
 */
struct PlaybackConfig {
    bool loop = false;
    aui::float_within_0_1 volume = 1.f;
};
