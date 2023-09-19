#pragma once

#include "AUI/Common/AByteBuffer.h"

struct ACodedFrame {
    AByteBuffer frameData;
    int64_t timecode;
};
