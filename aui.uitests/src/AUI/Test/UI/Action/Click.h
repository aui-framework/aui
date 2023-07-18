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


template<AInput::Key Button = AInput::LBUTTON>
struct ViewActionMousePress {
    AOptional<glm::ivec2> position;
    ViewActionMousePress() = default;

    ViewActionMousePress(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : view->getSize() / 2);
        auto window = view->getWindow();
        AInput::overrideStateForTesting(Button, true);
        window->onPointerPressed({coords, Button});
        uitest::frame();
    }
};

template<AInput::Key Button = AInput::LBUTTON>
struct ViewActionMouseRelease {
    AOptional<glm::ivec2> position;
    ViewActionMouseRelease() = default;

    ViewActionMouseRelease(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : view->getSize() / 2);
        auto window = view->getWindow();
        AInput::overrideStateForTesting(Button, false);
        window->onPointerReleased({coords, Button});
        uitest::frame();
    }
};


template<AInput::Key Button = AInput::LBUTTON>
struct ViewActionClick {
    AOptional<glm::ivec2> position;
    ViewActionClick() = default;

    ViewActionClick(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : view->getSize() / 2);
        auto window = view->getWindow();
        AInput::overrideStateForTesting(Button, true);
        window->onPointerPressed({coords, Button});
        uitest::frame();
        AInput::overrideStateForTesting(Button, false);
        window->onPointerReleased({coords, Button});
        uitest::frame();
    }
};

using mousePress = ViewActionMousePress<>;
using mouseRelease = ViewActionMouseRelease<>;
using click = ViewActionClick<>;

using mousePressRight = ViewActionMousePress<AInput::RBUTTON>;
using mouseReleaseRight = ViewActionMouseRelease<AInput::RBUTTON>;
using clickRight = ViewActionClick<AInput::RBUTTON>;
