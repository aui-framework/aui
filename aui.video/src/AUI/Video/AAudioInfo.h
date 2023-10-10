#pragma once

#include "AUI/Common/AByteBuffer.h"
#include "AUI/Audio/Codec.h"

struct AAudioInfo {
    AByteBuffer header;
    aui::audio::Codec codec = aui::audio::Codec::UNKNOWN;
};
