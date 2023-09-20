#pragma once

#include "AUI/Video/IVideoParser.h"
#include "IFrameDecoder.h"
#include "AUI/Util/AFunctionQueue.h"
#include "AUI/Thread/AFuture.h"
#include "AUI/Util/ARaiiHelper.h"

class AAsyncVideoProcessor : public AObject {
public:
    AAsyncVideoProcessor(_<IVideoParser> parser, _<IFrameDecoder> decoder);

    void run();

    _<AAsyncVideoProcessor> sharedPtr() {
        return _cast<AAsyncVideoProcessor>(AObject::sharedPtr());
    }

    bool hasFinished() {
        return mHasFinished;
    }

    AOptional<AFrame> nextFrame();

private:
    static constexpr size_t READY_FRAMES_MAX_SIZE = 10;
    class ReadyFramesQueue {
    public:
        AOptional<AFrame> pop() {
            std::unique_lock lock(mSync);

            if (mReadyFrames.empty()) {
                return std::nullopt;
            }

            ARaiiHelper helper = [this]() {
                mCV.notify_all();
            };

            return mReadyFrames.popOrGenerate([]() -> AFrame { return {}; });
        }

        void push(AFrame frame) {
            std::unique_lock lock(mSync);

            if (mReadyFrames.size() >= READY_FRAMES_MAX_SIZE) {
                mCV.wait(lock);
            }

            mReadyFrames << std::move(frame);
        }

    private:
        AConditionVariable mCV;
        AMutex mSync;
        AQueue<AFrame> mReadyFrames;
    };

    ReadyFramesQueue mReadyFrames;
    _<IVideoParser> mParser;
    _<IFrameDecoder> mDecoder;
    _<AThread> mParserThread;
    _<AThread> mDecoderThread;

    bool mHasFinished = false;

    void setupCallbacks();
};
