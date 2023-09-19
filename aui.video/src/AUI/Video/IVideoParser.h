#pragma once

#include "AUI/Common/AObject.h"
#include "AUI/Common/ASignal.h"
#include "AUI/Video/ACodedFrame.h"
#include "AVideoInfo.h"

class IVideoParser : public AObject {
public:
    virtual void run() = 0;

signals:
    emits<AVideoInfo> infoParsed;
    emits<ACodedFrame> frameParsed;
    emits<> finished;
};
