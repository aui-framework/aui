/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Platform/AWindow.h>

struct ViewActionDoubleClick {
    AOptional<glm::ivec2> position;
    ViewActionDoubleClick() = default;

    ViewActionDoubleClick(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : view->getSize() / 2);
        auto window = view->getWindow();
        window->onPointerPressed({coords, APointerIndex::button(AInput::LBUTTON)});
        uitest::frame();
        window->onPointerReleased({coords, APointerIndex::button(AInput::LBUTTON)});
        uitest::frame();
        window->onPointerPressed({coords, APointerIndex::button(AInput::LBUTTON)});
        uitest::frame();
        window->onPointerReleased({coords, APointerIndex::button(AInput::LBUTTON)});
        uitest::frame();
        window->onPointerDoubleClicked({coords, APointerIndex::button(AInput::LBUTTON)});
        uitest::frame();
    }
};

using doubleClick = ViewActionDoubleClick;