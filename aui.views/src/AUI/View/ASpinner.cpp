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
// Created by Alex2772 on 12/27/2021.
//

#include "ASpinner.h"

#include <AUI/Render/Render.h>

void ASpinner::render() {
    using namespace std::chrono;
    Render::translate(glm::vec2(getSize()) / 2.f);
    Render::rotate(float(
            duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() % 1000) / 1000.f * 2 * glm::pi<float>());
    Render::translate(-glm::vec2(getSize()) / 2.f);
    AView::render();
    redraw();
}
