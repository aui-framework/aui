#pragma once

#define COBJMACROS
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "AudioSession.h"
#include "AUI/Audio/Sound/ISoundStream.h"

class AudioSessionsManager {
public:
    static AudioSessionsManager& instance() {
        static AudioSessionsManager session;
        return session;
    }

    _<AudioSession> createNewSession(_<ISoundStream> stream);

private:
    static constexpr CLSID MM_DEVICE_ENUMERATOR_CLSID = __uuidof(MMDeviceEnumerator);
    static constexpr IID IMM_DEVICES_ENUMERATOR_IID = __uuidof(IMMDeviceEnumerator);

    IMMDevice* mOutputDevice = nullptr;
    IMMDeviceEnumerator* mDeviceEnumerator = nullptr;

    AudioSessionsManager();
    ~AudioSessionsManager();

};