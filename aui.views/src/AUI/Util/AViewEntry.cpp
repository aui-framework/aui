// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 12/31/2021.
//

#include "AViewEntry.h"

glm::ivec2 AViewEntry::getSize() {
    return {mView->getMinimumWidth() + mView->getMargin().horizontal(), mView->getMinimumHeight(
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
