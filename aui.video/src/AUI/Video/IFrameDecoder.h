#pragma once

#include "AUI/Video/AVideoFrame.h"
#include "AUI/Video/ACodedFrame.h"
#include "Codec.h"

class IFrameDecoder {
public:
    virtual AVideoFrame decode(const ACodedFrame& frame) = 0;
    static _<IFrameDecoder> fromCodec(aui::video::Codec codec);
};
