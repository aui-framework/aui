#include "AudioSession.h"
#include "AssertOkHelper.h"

AudioSession::AudioSession(_<ISoundStream> stream, IMMDevice *device) : mStream(std::move(stream)) {
    ASSERT_OK device->Activate(IAUDIO_CLIENT_IID,
                               CLSCTX_ALL,
                               nullptr,
                               reinterpret_cast<void**>(&mAudioClient));
    ASSERT_OK mAudioClient->GetMixFormat(&mMixFormat);
    mAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                             0,
                             REFTIMES_PER_SEC,
                             0,
                             mMixFormat,
                             nullptr);
    {
        UINT32 bufferSize;
        ASSERT_OK mAudioClient->GetBufferSize(&bufferSize);
        mBufferSize = static_cast<size_t>(bufferSize);
    }



}

void AudioSession::play() {
    if (!mSessionThread) {
        mSessionThread = _new<AThread>([self = shared_from_this(), soundStream = mStream, mixFormat = mMixFormat]() {
            for (size_t bytesRead = -1; bytesRead; ) {


                AThread::interruptionPoint();
            }
        });
        mSessionThread->start();
    }
}

void AudioSession::pause() {
    if (mSessionThread) {
        mSessionThread->interrupt();
        mSessionThread.reset();
    }
}
