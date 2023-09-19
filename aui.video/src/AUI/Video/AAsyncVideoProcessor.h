#pragma once

#include "AUI/Video/IVideoParser.h"
#include "AUI/Video/Codec/IFrameDecoder.h"

class AAsyncVideoProcessor {
public:
    AAsyncVideoProcessor(_<IVideoParser> parser, _<IFrameDecoder> decoder);

    void run();
private:
    _<IVideoParser> mParser;
    _<IFrameDecoder> mDecoder;
};
