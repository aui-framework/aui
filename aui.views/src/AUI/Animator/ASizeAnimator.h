// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
// Created by alex2 on 27.08.2020.
//

#pragma once

#include "AAnimator.h"

class API_AUI_VIEWS ASizeAnimator: public AAnimator {
private:
    glm::ivec2 mBeginSize;
    glm::ivec2 mEndSize;

public:
    ASizeAnimator(const glm::ivec2& beginSize = {0, 0}, const glm::ivec2& endSize = {0, 0});

    void doAnimation(AView* view, float theta) override;
};

