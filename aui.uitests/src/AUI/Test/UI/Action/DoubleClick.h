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

#pragma once

#include <AUI/Platform/AWindow.h>

struct ViewActionDoubleClick {
    AOptional<glm::ivec2> position;
    ViewActionDoubleClick() = default;

    ViewActionDoubleClick(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : view->getSize() / 2);
        auto window = view->getWindow();
        window->onMousePressed(coords, AInput::LBUTTON);
        uitest::frame();
        window->onMouseReleased(coords, AInput::LBUTTON);
        uitest::frame();
        window->onMousePressed(coords, AInput::LBUTTON);
        uitest::frame();
        window->onMouseReleased(coords, AInput::LBUTTON);
        uitest::frame();
        window->onMouseDoubleClicked(coords, AInput::LBUTTON);
        uitest::frame();
    }
};

using doubleClick = ViewActionDoubleClick;