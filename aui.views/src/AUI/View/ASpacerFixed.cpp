#include "ASpacerFixed.h"

int ASpacerFixed::getContentMinimumWidth(ALayoutDirection layout) {
    if (layout == ALayoutDirection::HORIZONTAL) return mSpace;
    return 0;
}

int ASpacerFixed::getContentMinimumHeight(ALayoutDirection layout) {
    if (layout == ALayoutDirection::VERTICAL) return mSpace;
    return 0;
}

bool ASpacerFixed::consumesClick(const glm::ivec2& pos) {
    return false;
}
