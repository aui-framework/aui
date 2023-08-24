#pragma once

#include "AUI/Platform/AWindow.h"
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
        auto w = dynamic_cast<AWindow*>(AWindow::current());
        auto handle = w->nativeHandle();
        ASSERT_OK mDirectSound->SetCooperativeLevel(handle, DSSCL_PRIORITY);
    }

    ~DirectSound() {
        AUI_NULLSAFE(mDirectSound)->Release();
    }

    IDirectSound8* mDirectSound;
};
