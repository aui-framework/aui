#pragma once

#include "AUI/Video/Codec.h"

struct AVideoInfo {
    size_t width = 0;
    size_t height = 0;
    aui::video::Codec codec = aui::video::Codec::UNKNOWN;
};
