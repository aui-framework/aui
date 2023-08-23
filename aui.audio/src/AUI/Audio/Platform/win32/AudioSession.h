#pragma once

#define COBJMACROS
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "AUI/Audio/Sound/ISoundStream.h"

class AudioSession : std::enable_shared_from_this<AudioSession> {
public:
    AudioSession(_<ISoundStream> stream, IMMDevice* device);

    void play();

    void pause();

private:
    IAudioClient* mAudioClient = nullptr;
    IAudioRenderClient* mAudioRenderClient = nullptr;
    WAVEFORMATEX* mMixFormat = nullptr;

    _<ISoundStream> mStream;
    _<AThread> mSessionThread;
    size_t mBufferSize;

    static constexpr IID IAUDIO_CLIENT_IID = __uuidof(IAudioClient);
    static constexpr IID IAUDIO_RENDERER_CLIENT_IID = __uuidof(IAudioRenderClient);
    static constexpr REFERENCE_TIME REFTIMES_PER_SEC = 10000000;
    static constexpr REFERENCE_TIME REFTIMES_PER_MILLISEC = 10000;
};
