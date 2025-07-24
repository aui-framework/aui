#include "DirectSoundAudioPlayer.h"
#include "AUI/Thread/IEventLoop.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Audio/AAudioMixer.h"
#include "AUI/Traits/memory.h"
#include <dsound.h>


#define ASSERT_OK AssertOkHelper{} +
struct AssertOkHelper {
    void operator+(HRESULT r) const {
        AUI_ASSERT(r == DS_OK);
    }
};

class DirectSound {
public:
    static DirectSound& instance() {
        static DirectSound ds;
        return ds;
    }

    const auto& thread() {
        return mThread;
    }

    auto& mixer() {
        return mMixer;
    }

private:
    static constexpr int BUFFER_DURATION_SEC = 2;
    static constexpr int UPLOADS_PER_SEC = 20;
    static constexpr int EVENTS_CNT = BUFFER_DURATION_SEC * UPLOADS_PER_SEC;
    static_assert(BUFFER_DURATION_SEC >= 2 && "Buffer duration assumes to be greater than 1");

    DirectSound() {
        ASSERT_OK DirectSoundCreate8(nullptr, &mDirectSound, nullptr);
        ASSERT_OK mDirectSound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
        setupSecondaryBuffer();
        setupReachPointEvents();
        setupBufferThread();
        uploadBlock(0);
        ASSERT_OK mSoundBufferInterface->Play(0, 0, DSBPLAY_LOOPING);
    }

    ~DirectSound() {
        ASSERT_OK mSoundBufferInterface->Stop();

        while (mThreadIsActive) {
            for (int i = 0; i < EVENTS_CNT; i++) {
                ResetEvent(mEvents[i]);
            }
            SetEvent(mEvents[EVENTS_CNT]);
        }

        for (auto& mEvent : mEvents)
            CloseHandle(mEvent);
        mThread->join();
        mThread = nullptr;
        clearBuffer();
        AUI_NULLSAFE(mDirectSound)->Release();
    }

    void setupReachPointEvents() {
        ASSERT_OK mSoundBufferInterface->QueryInterface(IID_IDirectSoundNotify8, (void**) &mNotifyInterface);
        mEvents[EVENTS_CNT] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        mEvents[EVENTS_CNT + 1] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        for (int i = 0; i < EVENTS_CNT; i++) {
            mEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            mNotifyPositions[i].hEventNotify = mEvents[i];
            mNotifyPositions[i].dwOffset = i * mBytesPerSecond / UPLOADS_PER_SEC;
        }

        ASSERT_OK (mNotifyInterface->SetNotificationPositions(EVENTS_CNT, mNotifyPositions));
    }

    void setupBufferThread() {
        mThread = _new<AThread>([this] {
            AThread::setName("aui.audio");
            mThreadIsActive = true;

            class AudioEventLoop: public IEventLoop {
            public:
                AudioEventLoop(DirectSound& parent): mParent(parent) {}
                void notifyProcessMessages() override {
                    SetEvent(mParent.mEvents[EVENTS_CNT + 1]);
                }

                void loop() override {
                    while (mParent.mThreadIsActive) {
                        mParent.onAudioReachCallbackPoint();
                    }
                }

            private:
                DirectSound& mParent;
            } eventLoop(*this);
            IEventLoop::Handle h(&eventLoop);
            eventLoop.loop();

        });
        mThread->start();
    }

    void uploadBlock(DWORD blockIndex) {
        LPVOID buffer;
        DWORD bufferSize = mBytesPerSecond / UPLOADS_PER_SEC;
        DWORD offset = blockIndex * mBytesPerSecond / UPLOADS_PER_SEC;

        HRESULT result = mSoundBufferInterface->Lock(offset, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
        if (result == DSERR_BUFFERLOST) {
            mSoundBufferInterface->Restore();
            ASSERT_OK mSoundBufferInterface->Lock(offset, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
        }

        bufferSize = mixer().readSoundData(std::span(reinterpret_cast<std::byte*>(buffer), bufferSize));
        ASSERT_OK mSoundBufferInterface->Unlock(buffer, bufferSize, nullptr, 0);
    }

    void onAudioReachCallbackPoint() {
        DWORD waitResult;
        waitResult = WaitForMultipleObjects(EVENTS_CNT + 1, mEvents, FALSE, INFINITE);
        while(waitResult != WAIT_FAILED) {
            DWORD eventIndex = waitResult - WAIT_OBJECT_0;
            if (eventIndex == EVENTS_CNT) {
                mIsPlaying = false;
                mThreadIsActive = false;
                return;
            } else if (eventIndex == EVENTS_CNT + 1) {
                ResetEvent(mEvents[EVENTS_CNT + 1]);
                AThread::processMessages();
            } else {
                uploadBlock((eventIndex + 1) % EVENTS_CNT);
            }
            waitResult = WaitForMultipleObjects(EVENTS_CNT + 2, mEvents, FALSE, INFINITE);
        }
    }

    void clearBuffer() {
        LPVOID buffer;
        DWORD bufferSize = BUFFER_DURATION_SEC * mBytesPerSecond;
        ASSERT_OK mSoundBufferInterface->Lock(0, bufferSize, &buffer, &bufferSize, nullptr, nullptr, 0);
        std::memset(buffer, 0, bufferSize);
        ASSERT_OK mSoundBufferInterface->Unlock(buffer, bufferSize, nullptr, 0);
    }


    static constexpr DWORD SAMPLE_RATE = 44100;
    static constexpr WORD BITS_PER_SAMPLE = 24;
    static constexpr WORD CHANNELS_COUNT = 2;

    void setupSecondaryBuffer() {
        DSBUFFERDESC format;
        WAVEFORMATEX waveFormat;
        IDirectSoundBuffer* buffer;

        waveFormat.cbSize = sizeof(waveFormat);
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.wBitsPerSample = BITS_PER_SAMPLE;
        waveFormat.nChannels = CHANNELS_COUNT;
        waveFormat.nSamplesPerSec = SAMPLE_RATE;
        waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);

        mBytesPerSecond = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.nAvgBytesPerSec = mBytesPerSecond;

        aui::zero(format);
        format.dwSize = sizeof(format);
        format.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
        format.dwBufferBytes = BUFFER_DURATION_SEC * mBytesPerSecond;
        format.lpwfxFormat = &waveFormat;


        ASSERT_OK mDirectSound->CreateSoundBuffer(&format, &buffer, nullptr);
        ASSERT_OK buffer->QueryInterface(IID_IDirectSoundBuffer8, reinterpret_cast<void**>(&mSoundBufferInterface));
        buffer->Release();
    }

    IDirectSound8* mDirectSound = nullptr;
    IDirectSoundBuffer8* mSoundBufferInterface = nullptr;
    IDirectSoundNotify8* mNotifyInterface = nullptr;
    DSBPOSITIONNOTIFY mNotifyPositions[EVENTS_CNT];
    HANDLE mEvents[EVENTS_CNT + 2];
    _<AThread> mThread;
    bool mThreadIsActive = false;
    bool mIsPlaying = false;
    DWORD mBytesPerSecond;
    AAudioMixer mMixer;
};

void DirectSoundAudioPlayer::playImpl() {
    DirectSound::instance().thread()->enqueue([self = aui::ptr::shared_from_this(this)]() mutable {
        self->initializeIfNeeded();
        DirectSound::instance().mixer().addSoundSource(std::move(self));
    });
}

void DirectSoundAudioPlayer::pauseImpl() {
    DirectSound::instance().thread()->enqueue([self = aui::ptr::shared_from_this(this)]() {
        DirectSound::instance().mixer().removeSoundSource(self);
    });
}

void DirectSoundAudioPlayer::stopImpl() {
    DirectSound::instance().thread()->enqueue([self = aui::ptr::shared_from_this(this)]() {
        DirectSound::instance().mixer().removeSoundSource(self);
        self->reset();
    });
}

void DirectSoundAudioPlayer::onLoopSet() {

}

void DirectSoundAudioPlayer::onVolumeSet() {

}
