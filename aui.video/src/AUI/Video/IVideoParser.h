#pragma once

#include "AUI/Common/AObject.h"
#include "AUI/Common/ASignal.h"
#include "AUI/Video/ACodedFrame.h"
#include "AVideoInfo.h"
#include "Codec.h"
#include "AUI/Audio/Codec.h"

class IVideoParser : public AObject {
public:
    virtual void run() = 0;

signals:
    emits<AVideoInfo> videoInfoParsed;
    emits<aui::video::Codec> videoCodecParsed;
    emits<aui::audio::Codec> audioCodecParsed;
    emits<ACodedFrame> videoFrameParsed;
    emits<ACodedFrame> audioFrameParsed;
    emits<> finished;
};