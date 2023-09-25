#include "DirectSoundAudioPlayer.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "DirectSound.h"

_<IAudioPlayer> IAudioPlayer::fromSoundStream(_<ISoundInputStream> stream) {
    auto result = _new<DirectSoundAudioPlayer>();
    result->setSource(std::move(stream));
    return result;
}

struct DirectSoundAudioPlayer::Private {
    IDirectSoundBuffer8* mSoundBufferInterface;
    IDirectSoundNotify8* mNotifyInterface;
    DSBPOSITIONNOTIFY mNotifyPositions[BUFFER_DURATION_SEC];
};

DirectSoundAudioPlayer::~DirectSoundAudioPlayer() {
    stop();
}


void DirectSoundAudioPlayer::playImpl() {
    setupReachPointEvents();
    setupBufferThread();
    uploadBlock(0);
    ASSERT_OK mPrivate->mSoundBufferInterface->Play(0, 0, DSBPLAY_LOOPING);
}

void DirectSoundAudioPlayer::pauseImpl() {
    ASSERT_OK mPrivate->mSoundBufferInterface->Stop();
}

void DirectSoundAudioPlayer::stopImpl() {
    ASSERT_OK mPrivate->mSoundBufferInterface->Stop();

    while (mThreadIsActive) {
        for (int i = 0; i < BUFFER_DURATION_SEC; i++)
            ResetEvent(mEvents[i]);
        SetEvent(mEvents[BUFFER_DURATION_SEC]);
    }

    for (auto& mEvent : mEvents)
        CloseHandle(mEvent);
    CloseHandle(mThread);
    clearBuffer();
    source()->rewind();
}

void DirectSoundAudioPlayer::uploadBlock(DWORD blockIndex) {
    LPVOID buffer;
    DWORD bufferSize = mBytesPerSecond;
    DWORD offset = blockIndex * mBytesPerSecond;

    HRESULT result = mPrivate->mSoundBufferInterface->Lock(offset, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
    if (result == DSERR_BUFFERLOST) {
        mPrivate->mSoundBufferInterface->Restore();
        ASSERT_OK mPrivate->mSoundBufferInterface->Lock(offset, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
    }

    bufferSize = source()->read(static_cast<char*>(buffer), bufferSize);
    ASSERT_OK mPrivate->mSoundBufferInterface->Unlock(buffer, bufferSize, nullptr, 0);
    if (bufferSize == 0) {
        if (loop()) {
            source()->rewind();
        }
        else {
            std::memset(buffer, 0, mBytesPerSecond - bufferSize);
            onFinished();
        }
    } else if (bufferSize < mBytesPerSecond) {
        onFinished();
    }
}

void DirectSoundAudioPlayer::clearBuffer() {
    LPVOID buffer;
    DWORD bufferSize = BUFFER_DURATION_SEC * mBytesPerSecond;
    ASSERT_OK mPrivate->mSoundBufferInterface->Lock(0, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
    std::memset(buffer, 0, bufferSize);
    ASSERT_OK mPrivate->mSoundBufferInterface->Unlock(buffer, bufferSize, nullptr, 0);
}

void DirectSoundAudioPlayer::setupBufferThread() {
    DWORD threadId;
    mThread = CreateThread(nullptr,
                           0,
                           (LPTHREAD_START_ROUTINE) bufferThread,
                           (void *) this,
                           0,
                           &threadId);
}

DWORD WINAPI DirectSoundAudioPlayer::bufferThread(void *lpParameter) {
    auto audio = reinterpret_cast<DirectSoundAudioPlayer*>(lpParameter);
    while(true) {
        audio->onAudioReachCallbackPoint();
    }
}

void DirectSoundAudioPlayer::onAudioReachCallbackPoint() {
    DWORD waitResult;
    waitResult = WaitForMultipleObjects(BUFFER_DURATION_SEC, mEvents, FALSE, INFINITE);
    while(waitResult != WAIT_FAILED) {
        DWORD eventIndex = waitResult - WAIT_OBJECT_0;
        if (eventIndex != BUFFER_DURATION_SEC) {
            uploadBlock((eventIndex + 1) % BUFFER_DURATION_SEC);
            waitResult = WaitForMultipleObjects(BUFFER_DURATION_SEC, mEvents, FALSE, INFINITE);
            continue;
        }

        mIsPlaying = false;
        mThreadIsActive = false;
        ExitThread(0);
    }
}

void DirectSoundAudioPlayer::setupReachPointEvents() {
    ASSERT_OK mPrivate->mSoundBufferInterface->QueryInterface(IID_IDirectSoundNotify8, (void**) &mPrivate->mNotifyInterface);
    mEvents[BUFFER_DURATION_SEC] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    for (int i = 0; i < BUFFER_DURATION_SEC; i++) {
        mEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        mPrivate->mNotifyPositions[i].hEventNotify = mEvents[i];
        mPrivate->mNotifyPositions[i].dwOffset = i * mBytesPerSecond + mBytesPerSecond / 2;
    }

    ASSERT_OK (mPrivate->mNotifyInterface->SetNotificationPositions(BUFFER_DURATION_SEC, mPrivate->mNotifyPositions));
}

void DirectSoundAudioPlayer::setupSecondaryBuffer() {
    DSBUFFERDESC format;
    WAVEFORMATEX waveFormat;
    IDirectSoundBuffer* buffer;

    auto info = source()->info();
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
    format.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME;
    format.dwBufferBytes = BUFFER_DURATION_SEC * mBytesPerSecond;
    format.lpwfxFormat = &waveFormat;


    ASSERT_OK DirectSound::instance()->CreateSoundBuffer(&format, &buffer, nullptr);
    ASSERT_OK buffer->QueryInterface(IID_IDirectSoundBuffer8, reinterpret_cast<void**>(&mPrivate->mSoundBufferInterface));
    buffer->Release();
}
void DirectSoundAudioPlayer::onSourceSet() {
    setupSecondaryBuffer();
}

void DirectSoundAudioPlayer::onLoopSet() {
}

void DirectSoundAudioPlayer::onVolumeSet() {
    AUI_NULLSAFE(mPrivate->mSoundBufferInterface)->SetVolume(
            static_cast<LONG>(glm::clamp(AMPLITUDE_DB * std::log10(float(volume()) / 256.f), MIN_VALUE_DB, 0.f)));
}
