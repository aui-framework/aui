#pragma once

#include "AUI/Video/AFrame.h"
#include "AUI/Common/AQueue.h"
#include "AUI/Common/AOptional.h"
#include <mutex>

class AFrame;

class AWebmFrameBuffer {
public:
    bool isFrameAvailable();

    AOptional<AFrame> extractFrame();

    void enqueueFrame(AFrame frame);

    AOptional<int32_t> nextFrameTimecode();

private:
    AQueue<AFrame> mFrameBuffer;
    std::mutex mMutex;
};
