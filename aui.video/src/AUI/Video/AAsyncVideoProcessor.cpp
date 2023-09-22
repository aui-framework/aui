#include "AAsyncVideoProcessor.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Video/IVideoParser.h"
#include "AUI/Video/IFrameDecoder.h"
#include "AUI/Thread/AEventLoop.h"

AAsyncVideoProcessor::AAsyncVideoProcessor(_<IVideoParser> parser, _<IFrameDecoder> decoder) :
            mParser(std::move(parser)), mDecoder(std::move(decoder)) {
    assert(mParser != nullptr);
}

void AAsyncVideoProcessor::run() {
    assert(("shouldn't be ran in the second time", mParserThread == nullptr));
    setupCallbacks();
    mParserThread = _new<AThread>([self = sharedPtr()]() {
         self->mParser->run();
    });
    mParserThread->start();

    mDecoderThread = _new<AThread>([self = sharedPtr()]() {
          AEventLoop loop;
          IEventLoop::Handle handle(&loop);
          AThread::current()->getCurrentEventLoop()->loop();
    });
    mDecoderThread->start();
}

AOptional<AVideoInfo> AAsyncVideoProcessor::videoInfo() {
    return mVideoInfo;
}

AOptional<AAudioInfo> AAsyncVideoProcessor::audioInfo() {
    return mAudioInfo;
}

AOptional<AVideoFrame> AAsyncVideoProcessor::nextVideoFrame() {
    return mReadyVideoFrames.pop();
}

AOptional<AEncodedFrame> AAsyncVideoProcessor::nextAudioFrame() {
    return mReadyAudioFrames.pop();
}

const AOptional<AVideoInfo>& AAsyncVideoProcessor::videoInfo() const {
    return mVideoInfo;
}

const AOptional<AAudioInfo>& AAsyncVideoProcessor::audioInfo() const {
    return mAudioInfo;
}

void AAsyncVideoProcessor::setupCallbacks() {
    AObject::connect(mParser->videoFrameParsed, [self = sharedPtr()](AEncodedFrame frame) {
        self->mDecoderThread->enqueue([self, frame = std::move(frame)]() {
            try {
                self->mReadyVideoFrames.push(self->mDecoder->decode(frame));
            }
            catch(...) { }
        });
        self->mDecoderThread->getCurrentEventLoop()->notifyProcessMessages();
    });

    AObject::connect(mParser->audioInfoParsed, [self = sharedPtr()](AAudioInfo info) {
        self->mAudioInfo = std::move(info);
    });

    AObject::connect(mParser->videoInfoParsed, [self = sharedPtr()](const AVideoInfo& info) {
        self->mVideoInfo = info;
        if (!self->mDecoder) {
            self->mDecoder = IFrameDecoder::fromCodec(info.codec);
        }
    });

    AObject::connect(mParser->audioFrameParsed, [self = sharedPtr()](AEncodedFrame frame) {
        self->mReadyAudioFrames.push(std::move(frame));
    });

    AObject::connect(mParser->finished, [self = sharedPtr()]() {
        self->mHasFinished = true;
        static_cast<AEventLoop*>(self->mDecoderThread->getCurrentEventLoop())->stop();
    });
}
