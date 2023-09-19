#pragma once

#include "AUI/Video/AFrame.h"
#include "AUI/Common/AQueue.h"
#include "AUI/Common/AOptional.h"
#include <mutex>

class AFrame;

class AFrameBuffer {
public:
    bool isFrameAvailable();

    AOptional<AFrame> extractFrame();

    void enqueueFrame(AFrame frame);

    AOptional<int16_t> nextFrameTimecode();

private:
    AQueue<AFrame> mFrameBuffer;
    std::mutex mMutex;
};
