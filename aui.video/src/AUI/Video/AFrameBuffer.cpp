#include "AFrameBuffer.h"

bool AFrameBuffer::isFrameAvailable() {
    std::unique_lock lock(mMutex);
    return !mFrameBuffer.empty();
}

AOptional<AFrame> AFrameBuffer::extractFrame() {
    std::unique_lock lock(mMutex);
    if (mFrameBuffer.empty()) {
        return std::nullopt;
    }

    auto result = std::move(mFrameBuffer.front());
    mFrameBuffer.pop();
    return result;
}

void AFrameBuffer::enqueueFrame(AFrame frame) {
    std::unique_lock lock(mMutex);
    mFrameBuffer.push(std::move(frame));
}

AOptional<int16_t> AFrameBuffer::nextFrameTimecode() {
    std::unique_lock lock(mMutex);
    if (mFrameBuffer.empty()) {
        return std::nullopt;
    }

    return mFrameBuffer.front().timecode;
}
