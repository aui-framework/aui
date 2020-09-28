#include "ASpacer.h"

int ASpacer::getContentMinimumWidth() {
    return mMinimumSize.x;
}

int ASpacer::getContentMinimumHeight() {
    return mMinimumSize.y;
}

bool ASpacer::consumesClick(const glm::ivec2& pos) {
    return false;
}
