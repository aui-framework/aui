#include "AAsyncVideoProcessor.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Thread/IEventLoop.h"
#include "AUI/Thread/AEventLoop.h"

AAsyncVideoProcessor::AAsyncVideoProcessor(_<IVideoParser> parser, _<IFrameDecoder> decoder) :
            mParser(std::move(parser)), mDecoder(std::move(decoder)) {
    assert(mParser != nullptr);
}

void AAsyncVideoProcessor::run() {
    assert(("shouldn't be run in the second time", mParserThread == nullptr));
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

AOptional<AFrame> AAsyncVideoProcessor::nextFrame() {
    return mReadyFrames.pop();
}

void AAsyncVideoProcessor::setupCallbacks() {
    AObject::connect(mParser->videoFrameParsed, [self = sharedPtr()](ACodedFrame frame) {
        self->mDecoderThread->enqueue([self, frame = std::move(frame)]() {
            try {
                self->mReadyFrames.push(self->mDecoder->decode(frame));
            }
            catch(...) { }
        });
        self->mDecoderThread->getCurrentEventLoop()->notifyProcessMessages();
    });

    AObject::connect(mParser->finished, [self = sharedPtr()]() {
        self->mHasFinished = true;
        static_cast<AEventLoop*>(self->mDecoderThread->getCurrentEventLoop())->stop();
    });

    if (!mDecoder) {
        AObject::connect(mParser->videoCodecParsed, [self = sharedPtr()](aui::video::Codec codec) {
            if (!self->mDecoder) {
                self->mDecoder = IFrameDecoder::fromCodec(codec);
            }
        });
    }
}
