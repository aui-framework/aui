#pragma once

#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Util/APimpl.h"

class DirectSoundAudioPlayer : public IAudioPlayer {
public:
    using IAudioPlayer::IAudioPlayer;
private:
    void playImpl() override;
    void pauseImpl() override;
    void stopImpl() override;

    void onLoopSet() override;
    void onVolumeSet() override;
};
