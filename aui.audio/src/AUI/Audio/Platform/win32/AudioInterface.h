//
// Created by Alex2772 on 2/9/2022.
//

#include "AUI/Platform/AProgramModule.h"

#pragma once
#pragma comment(lib,"dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment( lib, "Winmm" )

#include <utility>
#include <dsound.h>
#include "AUI/Traits/memory.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Thread/AFuture.h"
#include "SDL_Audio_Base.h"
#include "AUI/Audio/Sound/Sound.h"

#define ASSERT_OK AssertOkHelper{} +
struct AssertOkHelper {
    void operator+(HRESULT r) const {
        assert(r == DS_OK);
    }
};

struct DirectSoundInstance {
    IDirectSound8* mDirectSound;
    DirectSoundInstance() {
        if (DirectSoundCreate8(nullptr, &mDirectSound, nullptr) != 0) {
            mDirectSound = nullptr;
            return;
        }

        auto w = dynamic_cast<AWindow*>(AWindow::current());
        auto handle = w->nativeHandle();
        ASSERT_OK mDirectSound->SetCooperativeLevel(handle, DSSCL_PRIORITY);
    }

    DirectSoundInstance(const DirectSoundInstance&) = delete;

    ~DirectSoundInstance() {
        nullsafe(mDirectSound)->Release();
    }
};

static IDirectSound8* getDirectSound() {
    static DirectSoundInstance ds;
    return ds.mDirectSound;
}

class AudioInterface: public SDL_Audio_Base {
public:
    AudioInterface(std::shared_ptr<Audio::SoundStream> sound) : mSound(std::move(sound)){
        setupSecondaryBuffer();
    }

    bool release() override {
        mSoundBufferInterface->Release();
        mNotifyInterface->Release();
        return true;
    }

    bool play() override {
        stop();
        setupReachPointEvents();
        setupBufferThread();
        uploadNextBlock(BUFFER_DURATION_SEC);
        resume();
        mIsPlaying = true;
        return true;
    }

    bool fadeIn(int ms) override {
        return true;
    }

    bool fadeOut(int ms) override {
        return true;
    }

    bool stop() override {
        ASSERT_OK mSoundBufferInterface->Stop();

        while (mThreadIsActive) {
            for (int i = 0; i < BUFFER_DURATION_SEC; i++)
                ResetEvent(mEvents[i]);
            SetEvent(mEvents[BUFFER_DURATION_SEC]);
        }

        for (auto& mEvent : mEvents)
            CloseHandle(mEvent);
        CloseHandle(mThread);

        clearBuffer();
        rewind();
        return true;
    }

    bool pause() override {
        ASSERT_OK mSoundBufferInterface->Stop();
        mIsPlaying = false;
        return true;
    }

    bool resume() override {
        ASSERT_OK mSoundBufferInterface->Play(0, 0, DSBPLAY_LOOPING);
        mIsPlaying = true;
        return true;
    }

    bool rewind() override {
        ASSERT_OK mSoundBufferInterface->SetCurrentPosition(0);
        mSound->rewind();
        return true;
    }

    DWORD status() const {
        DWORD status;
        ASSERT_OK mSoundBufferInterface->GetStatus(&status);
        return status;
    }

    bool is_playing() override {
        return mIsPlaying;
    }

    bool is_paused() override {
        return !mIsPlaying;
    }

private:
    static constexpr int BUFFER_DURATION_SEC = 2;
    static_assert(BUFFER_DURATION_SEC >= 2 && "Buffer duration assumes to be greater than 1");

    HANDLE mEvents[BUFFER_DURATION_SEC + 1];
    DSBPOSITIONNOTIFY mNotifyPositions[BUFFER_DURATION_SEC];
    HANDLE mThread;
    bool mThreadIsActive = false;

    IDirectSoundBuffer8* mSoundBufferInterface;
    IDirectSoundNotify8* mNotifyInterface;
    std::shared_ptr<Audio::SoundStream> mSound;

    bool mIsPlaying = false;
    int bytesPerSecond;

    void uploadNextBlock(DWORD reachedPointIndex) {
        LPVOID buffer;
        DWORD bufferSize = bytesPerSecond;
        DWORD offset = ((reachedPointIndex + 1) % BUFFER_DURATION_SEC) * bytesPerSecond;

        HRESULT result = mSoundBufferInterface->Lock(offset, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
        if (result == DSERR_BUFFERLOST) {
            mSoundBufferInterface->Restore();
            ASSERT_OK mSoundBufferInterface->Lock(offset, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
        }

        bufferSize = mSound->read(static_cast<char*>(buffer), bufferSize);
        ASSERT_OK mSoundBufferInterface->Unlock(buffer, bufferSize, nullptr, 0);
        if (bufferSize == 0) {
            std::memset(buffer, 0, bytesPerSecond - bufferSize);
            stop();
        } else if (bufferSize < bytesPerSecond) {
            stop();
        }
    }

    void clearBuffer() {
        LPVOID buffer;
        DWORD bufferSize = BUFFER_DURATION_SEC * bytesPerSecond;
        ASSERT_OK mSoundBufferInterface->Lock(0, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
        std::memset(buffer, 0, bufferSize);
        ASSERT_OK mSoundBufferInterface->Unlock(buffer, bufferSize, nullptr, 0);
    }

    void setupBufferThread() {
        DWORD threadId;
        mThread = CreateThread(nullptr,
                               0,
                               (LPTHREAD_START_ROUTINE) bufferThread,
                               (void *) this,
                               0,
                               &threadId);
    }

    [[noreturn]] static DWORD WINAPI bufferThread(void *lpParameter) {
        auto audio = reinterpret_cast<AudioInterface*>(lpParameter);
        while(true) {
            audio->onAudioReachCallbackPoint();
        }
    }

    void onAudioReachCallbackPoint() {
        DWORD waitResult;
        waitResult = WaitForMultipleObjects(BUFFER_DURATION_SEC, mEvents, FALSE, INFINITE);
        while(waitResult != WAIT_FAILED) {
            DWORD eventIndex = waitResult - WAIT_OBJECT_0;
            if (eventIndex != BUFFER_DURATION_SEC) {
                uploadNextBlock(eventIndex);
                waitResult = WaitForMultipleObjects(BUFFER_DURATION_SEC, mEvents, FALSE, INFINITE);
                continue;
            }

            mIsPlaying = false;
            mThreadIsActive = false;
            ExitThread(0);
        }
    }

    void setupReachPointEvents() {
        ASSERT_OK mSoundBufferInterface->QueryInterface(IID_IDirectSoundNotify8, (void**) &mNotifyInterface);
        mEvents[BUFFER_DURATION_SEC] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        for (int i = 0; i < BUFFER_DURATION_SEC; i++) {
            mEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            mNotifyPositions[i].hEventNotify = mEvents[i];
            mNotifyPositions[i].dwOffset = i * bytesPerSecond + bytesPerSecond / 2;
        }

        ASSERT_OK (mNotifyInterface->SetNotificationPositions(BUFFER_DURATION_SEC, mNotifyPositions));
    }

    void setupSecondaryBuffer() {
        DSBUFFERDESC format;
        WAVEFORMATEX waveFormat;
        IDirectSoundBuffer* buffer;

        auto info = mSound->info();
        waveFormat.cbSize = sizeof(waveFormat);
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.wBitsPerSample = info.bitsPerSample;
        waveFormat.nChannels = info.channelCount;
        waveFormat.nSamplesPerSec = info.sampleRate;
        waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);

        bytesPerSecond = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.nAvgBytesPerSec = bytesPerSecond;

        aui::zero(format);
        format.dwSize = sizeof(format);
        format.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
        format.dwBufferBytes = BUFFER_DURATION_SEC * bytesPerSecond;
        format.lpwfxFormat = &waveFormat;

        ASSERT_OK getDirectSound()->CreateSoundBuffer(&format, &buffer, nullptr);
        ASSERT_OK buffer->QueryInterface(IID_IDirectSoundBuffer8, (void**) &mSoundBufferInterface);
        buffer->Release();
    }
};