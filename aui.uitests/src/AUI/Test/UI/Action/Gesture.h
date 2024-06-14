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


struct ViewActionGestureLongPress {
    AOptional<glm::ivec2> position;
    ViewActionGestureLongPress() = default;

    ViewActionGestureLongPress(const glm::ivec2& position) : position(position) {}

    void operator()(const _<AView>& view) {
        auto coords = view->getPositionInWindow() + (position ? *position : view->getSize() / 2);
        auto window = view->getWindow();
        window->onGesture(coords, ALongPressEvent{});
        uitest::frame();
    }
};

using gestureLongPress = ViewActionGestureLongPress;