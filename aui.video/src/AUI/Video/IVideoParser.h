#pragma once

#include "AUI/Common/AObject.h"
#include "AUI/Common/ASignal.h"
#include "AUI/Video/AEncodedFrame.h"
#include "AVideoInfo.h"
#include "AAudioInfo.h"

class IVideoParser : public AObject {
public:
    virtual void run() = 0;

signals:
    emits<AVideoInfo> videoInfoParsed;
    emits<AAudioInfo> audioInfoParsed;
    emits<AEncodedFrame> videoFrameParsed;
    emits<AEncodedFrame> audioFrameParsed;
    emits<> finished;
};