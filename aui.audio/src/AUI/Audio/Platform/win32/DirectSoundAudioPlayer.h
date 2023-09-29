#pragma once

#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Util/APimpl.h"

class DirectSoundAudioPlayer : public IAudioPlayer {
public :
    ~DirectSoundAudioPlayer() override;

private:
    void playImpl() override;
    void pauseImpl() override;
    void stopImpl() override;

    void onSourceSet() override;
    void onLoopSet() override;
    void onVolumeSet() override;
};
