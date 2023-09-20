#pragma once

#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Util/APimpl.h"
#include <Windows.h>

class ADirectSoundAudioPlayer : public IAudioPlayer {
public :
    ~ADirectSoundAudioPlayer() override;

private:
    void playImpl() override;
    void pauseImpl() override;
    void stopImpl() override;

    void onSourceSet() override;
    void onLoopSet() override;
    void onVolumeSet() override;

    static constexpr int BUFFER_DURATION_SEC = 2;
    static_assert(BUFFER_DURATION_SEC >= 2 && "Buffer duration assumes to be greater than 1");
    static constexpr float MIN_VALUE_DB = -10000;
    static constexpr float AMPLITUDE_DB = 2000;

    HANDLE mEvents[BUFFER_DURATION_SEC + 1];
    HANDLE mThread;
    bool mThreadIsActive = false;

    struct Private;
    aui::fast_pimpl<Private, (sizeof(void*) + sizeof(long)) * (2 + BUFFER_DURATION_SEC), alignof(void*)> mPrivate;

    bool mIsPlaying = false;
    int mBytesPerSecond;

    void uploadNextBlock(DWORD reachedPointIndex);

    void clearBuffer();

    void setupBufferThread();

    [[noreturn]]
    static DWORD WINAPI bufferThread(void *lpParameter);

    void onAudioReachCallbackPoint();

    void setupReachPointEvents();

    void setupSecondaryBuffer();
};
