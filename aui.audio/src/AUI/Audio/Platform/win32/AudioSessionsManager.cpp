#include "AudioSessionsManager.h"
#include "AssertOkHelper.h"

AudioSessionsManager::AudioSessionsManager() {
    ASSERT_OK CoCreateInstance(MM_DEVICE_ENUMERATOR_CLSID,
                               nullptr,
                               CLSCTX_ALL,
                               IMM_DEVICES_ENUMERATOR_IID, reinterpret_cast<void**>(&mDeviceEnumerator)
                               );
    ASSERT_OK mDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &mOutputDevice);

}

AudioSessionsManager::~AudioSessionsManager() {
    AUI_NULLSAFE(mOutputDevice)->Release();
    AUI_NULLSAFE(mDeviceEnumerator)->Release();
}
