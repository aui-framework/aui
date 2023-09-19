#include "AAsyncVideoProcessor.h"

AAsyncVideoProcessor::AAsyncVideoProcessor(_<IVideoParser> parser, _<IFrameDecoder> decoder) :
            mParser(std::move(parser)), mDecoder(std::move(decoder)) {
    assert(mParser != nullptr);


}

void AAsyncVideoProcessor::run() {

}
