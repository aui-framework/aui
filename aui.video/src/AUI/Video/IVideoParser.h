#pragma once

#include "AUI/Common/AObject.h"
#include "AUI/Common/ASignal.h"
#include "AUI/Video/ACodedFrame.h"
#include "AVideoInfo.h"
#include "AUI/Video/Codec/Codec.h"

class IVideoParser : public AObject {
public:
    virtual void run() = 0;

signals:
    emits<AVideoInfo> infoParsed;
    emits<aui::video::Codec> codecParsed;
    emits<ACodedFrame> frameParsed;
    emits<> finished;
};
