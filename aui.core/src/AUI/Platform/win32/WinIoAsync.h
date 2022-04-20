#pragma once


#include <Windows.h>
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/ASignal.h"

class WinIoAsync {
public:
    WinIoAsync() = default;
    ~WinIoAsync() = default;


    void init(HANDLE fileHandle, std::function<void(const AByteBuffer&)> callback);

private:
    class Impl;
    _<Impl> mImpl;
};


