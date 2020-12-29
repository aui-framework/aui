#pragma once

#include "AFont.h"

struct AFont::Character {
    long c;
    size_t width, height;
    float advanceX, advanceY;
    _<glm::vec4> uv;
    float bearingX;
};
