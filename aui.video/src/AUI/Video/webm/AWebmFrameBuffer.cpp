//
// Created by ilyazavalov on 9/14/23.
//

#include "AWebmFrameBuffer.h"

bool AWebmFrameBuffer::isFrameAvailable() {
    std::unique_lock lock(mMutex);
    return !mFrameBuffer.empty();
}

AOptional<AFrame> AWebmFrameBuffer::extractFrame() {
    std::unique_lock lock(mMutex);
    if (mFrameBuffer.empty()) {
        return std::nullopt;
    }

    auto result = std::move(mFrameBuffer.front());
    mFrameBuffer.pop();
    return result;
}

void AWebmFrameBuffer::enqueueFrame(AFrame frame) {
    std::unique_lock lock(mMutex);
    mFrameBuffer.push(std::move(frame));
}

AOptional<int32_t> AWebmFrameBuffer::nextFrameTimecode() {
    std::unique_lock lock(mMutex);
    if (mFrameBuffer.empty()) {
        return std::nullopt;
    }

    return mFrameBuffer.front().timecode;
}
