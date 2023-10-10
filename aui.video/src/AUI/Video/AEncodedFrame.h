#pragma once

#include "AUI/Common/AByteBuffer.h"

struct AEncodedFrame {
    AByteBuffer frameData;
    int64_t timecode = 0;
};
