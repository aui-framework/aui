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

struct ViewActionKeyDownAndUp: ViewActionClick<> {
    AShortcut key;

    explicit ViewActionKeyDownAndUp(AShortcut key) : key(std::move(key)) {}

    void operator()(const _<AView>& view) {
        // click on it to acquire focus
        view->focus();

        auto window = view->getWindow();

        uitest::frame();
        for (const auto& k : key.getKeys()) {
            AInput::overrideStateForTesting(k, true);
            window->onKeyDown(k);
        }
        for (const auto& k : key.getKeys()) {
            AInput::overrideStateForTesting(k, false);
            window->onKeyUp(k);
        }
    }
};

using keyDownAndUp = ViewActionKeyDownAndUp;