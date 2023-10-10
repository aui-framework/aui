#pragma once

#include "AUI/Image/AImage.h"

struct AVideoFrame {
    AImage image;
    int64_t timecode = 0;
};
