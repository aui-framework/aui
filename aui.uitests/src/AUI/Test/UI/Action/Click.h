/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Platform/AWindow.h>


template<AInput::Key Button = AInput::LBUTTON>
struct ViewActionMousePress {
    AOptional<glm::ivec2> position;
    ViewActionMousePress() = default;

    ViewActionMousePress(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : (view->getSize() / 2));
        auto window = view->getWindow();
        AInput::overrideStateForTesting(Button, true);
        window->onPointerPressed({coords, APointerIndex::button(Button)});
        uitest::frame();
    }
};

template<AInput::Key Button = AInput::LBUTTON>
struct ViewActionMouseRelease {
    AOptional<glm::ivec2> position;
    ViewActionMouseRelease() = default;

    ViewActionMouseRelease(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : (view->getSize() / 2));
        auto window = view->getWindow();
        AInput::overrideStateForTesting(Button, false);
        window->onPointerReleased({coords, APointerIndex::button(Button)});
        uitest::frame();
    }
};


template<AInput::Key Button = AInput::LBUTTON>
struct ViewActionClick {
    AOptional<glm::ivec2> position;
    ViewActionClick() = default;

    ViewActionClick(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : (view->getSize() / 2));
        auto window = view->getWindow();
        AInput::overrideStateForTesting(Button, true);
        window->onPointerPressed({coords, APointerIndex::button(Button)});
        uitest::frame();
        AInput::overrideStateForTesting(Button, false);
        window->onPointerReleased({coords, APointerIndex::button(Button)});
        uitest::frame();
    }
};

using mousePress = ViewActionMousePress<>;
using mouseRelease = ViewActionMouseRelease<>;
using click = ViewActionClick<>;

using mousePressRight = ViewActionMousePress<AInput::RBUTTON>;
using mouseReleaseRight = ViewActionMouseRelease<AInput::RBUTTON>;
using clickRight = ViewActionClick<AInput::RBUTTON>;


struct ViewActionPointerPressed { // for the cases when mouse* actions is not enough
    AOptional<glm::ivec2> position;
    APointerIndex pointerIndex = APointerIndex::button(AInput::LBUTTON);

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : (view->getSize() / 2));
        auto window = view->getWindow();
        window->onPointerPressed({coords, pointerIndex});
        uitest::frame();
    }
};

struct ViewActionPointerReleased { // for the cases when mouse* actions is not enough
    AOptional<glm::ivec2> position;
    APointerIndex pointerIndex = APointerIndex::button(AInput::LBUTTON);

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : (view->getSize() / 2));
        auto window = view->getWindow();
        window->onPointerReleased({coords, pointerIndex});
        uitest::frame();
    }
};

struct ViewActionPointerMoved { // for the cases when mouse* actions is not enough
    AOptional<glm::ivec2> position;
    APointerIndex pointerIndex = APointerIndex::button(AInput::LBUTTON);

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : (view->getSize() / 2));
        auto window = view->getWindow();
        window->onPointerMove(coords, {pointerIndex});
        uitest::frame();
    }
};

using pointerPress = ViewActionPointerPressed;
using pointerRelease = ViewActionPointerPressed;
using pointerMove = ViewActionPointerMoved;
