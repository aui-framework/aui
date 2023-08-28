#include "AUI/Audio/AAudioPlayer.h"

void AAudioPlayer::playImpl() {
    setupReachPointEvents();
    setupBufferThread();
    uploadNextBlock(BUFFER_DURATION_SEC);
    ASSERT_OK mSoundBufferInterface->Play(0, 0, DSBPLAY_LOOPING);
}

void AAudioPlayer::pauseImpl() {
    ASSERT_OK mSoundBufferInterface->Stop();
}

void AAudioPlayer::stopImpl() {
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
    mSource->rewind();
}

void AAudioPlayer::uploadNextBlock(DWORD reachedPointIndex) {
    LPVOID buffer;
    DWORD bufferSize = mBytesPerSecond;
    DWORD offset = ((reachedPointIndex + 1) % BUFFER_DURATION_SEC) * mBytesPerSecond;

    HRESULT result = mSoundBufferInterface->Lock(offset, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
    if (result == DSERR_BUFFERLOST) {
        mSoundBufferInterface->Restore();
        ASSERT_OK mSoundBufferInterface->Lock(offset, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
    }

    bufferSize = mSource->read(static_cast<char*>(buffer), bufferSize);
    ASSERT_OK mSoundBufferInterface->Unlock(buffer, bufferSize, nullptr, 0);
    if (bufferSize == 0) {
        std::memset(buffer, 0, mBytesPerSecond - bufferSize);
        stop();
    } else if (bufferSize < mBytesPerSecond) {
        stop();
    }
}

void AAudioPlayer::clearBuffer() {
    LPVOID buffer;
    DWORD bufferSize = BUFFER_DURATION_SEC * mBytesPerSecond;
    ASSERT_OK mSoundBufferInterface->Lock(0, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
    std::memset(buffer, 0, bufferSize);
    ASSERT_OK mSoundBufferInterface->Unlock(buffer, bufferSize, nullptr, 0);
}

void AAudioPlayer::setupBufferThread() {
    DWORD threadId;
    mThread = CreateThread(nullptr,
                           0,
                           (LPTHREAD_START_ROUTINE) bufferThread,
                           (void *) this,
                           0,
                           &threadId);
}

DWORD WINAPI AAudioPlayer::bufferThread(void *lpParameter) {
    auto audio = reinterpret_cast<AAudioPlayer*>(lpParameter);
    while(true) {
        audio->onAudioReachCallbackPoint();
    }
}

void AAudioPlayer::onAudioReachCallbackPoint() {
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

void AAudioPlayer::setupReachPointEvents() {
    ASSERT_OK mSoundBufferInterface->QueryInterface(IID_IDirectSoundNotify8, (void**) &mNotifyInterface);
    mEvents[BUFFER_DURATION_SEC] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    for (int i = 0; i < BUFFER_DURATION_SEC; i++) {
        mEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        mNotifyPositions[i].hEventNotify = mEvents[i];
        mNotifyPositions[i].dwOffset = i * mBytesPerSecond + mBytesPerSecond / 2;
    }

    ASSERT_OK (mNotifyInterface->SetNotificationPositions(BUFFER_DURATION_SEC, mNotifyPositions));
}

void AAudioPlayer::setupSecondaryBuffer() {
    DSBUFFERDESC format;
    WAVEFORMATEX waveFormat;
    IDirectSoundBuffer* buffer;

    auto info = mSource->info();
    waveFormat.cbSize = sizeof(waveFormat);
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.wBitsPerSample = info.bitsPerSample();
    waveFormat.nChannels = info.channelCount;
    waveFormat.nSamplesPerSec = info.sampleRate;
    waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);

    mBytesPerSecond = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.nAvgBytesPerSec = mBytesPerSecond;

    aui::zero(format);
    format.dwSize = sizeof(format);
    format.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
    format.dwBufferBytes = BUFFER_DURATION_SEC * mBytesPerSecond;
    format.lpwfxFormat = &waveFormat;

    ASSERT_OK DirectSound::instance()->CreateSoundBuffer(&format, &buffer, nullptr);
    ASSERT_OK buffer->QueryInterface(IID_IDirectSoundBuffer8, reinterpret_cast<void**>(&mSoundBufferInterface));
    buffer->Release();
}

void AAudioPlayer::onSourceSet() {
    setupSecondaryBuffer();
}
