#pragma once

#include "AUI/Video/AFrame.h"
#include "AUI/Video/ACodedFrame.h"
#include "AUI/Video/Codec/Codec.h"

class IFrameDecoder {
public:
    virtual AFrame decode(const ACodedFrame& frame) = 0;
    static _<IFrameDecoder> fromCodec(aui::video::Codec codec);
};
