#pragma once

#include "AUI/Audio/IAudioPlayer.h"

class APulseAudioPlayer : public IAudioPlayer {
private:
    void playImpl() override;
    void pauseImpl() override;
    void stopImpl() override;

    void onSourceSet() override;
    void onLoopSet() override;
    void onVolumeSet() override;
};
