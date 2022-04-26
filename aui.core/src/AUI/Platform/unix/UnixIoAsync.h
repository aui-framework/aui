#pragma once


#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/ASignal.h"

class UnixIoAsync {
public:
    UnixIoAsync() = default;
    ~UnixIoAsync() = default;


    void init(int fileHandle, std::function<void(const AByteBuffer&)> callback);

private:
    class Impl;
    _<Impl> mImpl;
};


