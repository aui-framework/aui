#pragma once

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Winmm" )

#include <cassert>
#include <dsound.h>

#define ASSERT_OK AssertOkHelper{} +
struct AssertOkHelper {
    void operator+(HRESULT r) const {
        assert(r == DS_OK);
    }
};

class DirectSound {
public:
    static IDirectSound* instance() {
        static DirectSound ds;
        return ds.mDirectSound;
    }

private:
    DirectSound() {
        ASSERT_OK DirectSoundCreate8(nullptr, &mDirectSound, nullptr);
        ASSERT_OK mDirectSound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
    }

    ~DirectSound() {
        AUI_NULLSAFE(mDirectSound)->Release();
    }

    IDirectSound8* mDirectSound;
};
