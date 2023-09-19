#pragma once

#include "AUI/Video/IVideoParser.h"
#include "AUI/Video/Codec/IFrameDecoder.h"
#include "AUI/Util/AFunctionQueue.h"
#include "AUI/Thread/AFuture.h"

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
    _<IVideoParser> mParser;
    _<IFrameDecoder> mDecoder;
    AFunctionQueue mDecodingTasks;

    AFuture<AFrame> mFrameFuture;
    _<AThread> mParserThread;

    std::atomic_bool mHasFinished = false;

    void setupCallbacks();
};
