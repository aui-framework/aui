//
// Created by Alex2772 on 12/31/2021.
//

#include "AViewEntry.h"

glm::ivec2 AViewEntry::getSize() {
    return {mView->getMinimumWidth(ALayoutDirection::NONE) + mView->getMargin().horizontal(), mView->getMinimumHeight(
            ALayoutDirection::NONE) + mView->getMargin().vertical() };
}

void AViewEntry::setPosition(const glm::ivec2& position) {
    mView->setGeometry(position + glm::ivec2{mView->getMargin().left, mView->getMargin().top},
                       mView->getMinimumSize());

}

Float AViewEntry::getFloat() const {
    return Float::NONE;
}

AViewEntry::~AViewEntry() {

}
