#include "AAsyncVideoProcessor.h"
#include "AUI/Logging/ALogger.h"

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
}

AOptional<AFrame> AAsyncVideoProcessor::nextFrame() {
    if (mDecodingTasks.empty() || !mDecoder) {
        return std::nullopt;
    }

    try {
        mDecodingTasks.processSingle();
        return *mFrameFuture;
    }
    catch (const AException& e) {
        ALogger::err("webm") << "Failed to decode frame : " << e.what();
    }
    catch(...) { }

    return std::nullopt;
}

void AAsyncVideoProcessor::setupCallbacks() {
    AObject::connect(mParser->frameParsed, [self = sharedPtr()](ACodedFrame frame) {
         self->mDecodingTasks << [self = std::move(self), frame = std::move(frame)]() {
              self->mFrameFuture = asyncX [self = std::move(self), frame = std::move(frame)] {
                  return self->mDecoder->decode(frame);
              };
         };
    });

    AObject::connect(mParser->finished, [self = sharedPtr()]() {
        self->mHasFinished = true;
    });

    if (!mDecoder) {
        AObject::connect(mParser->codecParsed, [self = sharedPtr()](aui::video::Codec codec) {
            if (!self->mDecoder) {
                self->mDecoder = IFrameDecoder::fromCodec(codec);
            }
        });
    }
}
