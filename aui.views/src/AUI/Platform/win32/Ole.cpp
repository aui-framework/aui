//
// Created by Alex2772 on 9/15/2022.
//

#include <Windows.h>
#include <cassert>
#include "Ole.h"

Ole::Ole() {
    OleInitialize(0);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    assert(SUCCEEDED(hr));
}

Ole::~Ole() {
    CoUninitialize();
    OleUninitialize();
}

Ole& Ole::inst() {
    static Ole ole;
    return ole;
}
