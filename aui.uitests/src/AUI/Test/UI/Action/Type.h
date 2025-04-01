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

struct ViewActionType: ViewActionClick<> {
    AString text;

    explicit ViewActionType(const AString& text) : text(text) {}

    void operator()(const _<AView>& view) {
        view->focus();

        auto window = view->getWindow();

        uitest::frame();
        // type
        for (auto c : text) {
            window->onCharEntered(c);
            uitest::frame();
        }
    }
};

using type = ViewActionType;