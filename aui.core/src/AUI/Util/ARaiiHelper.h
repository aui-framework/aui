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

#include <AUI/Traits/concepts.h>

template<aui::invocable Callback>
class ARaiiHelper {
public:
    ARaiiHelper(Callback&& callback) noexcept: mCallback(std::move(callback)) {

    }
    ARaiiHelper(const ARaiiHelper&) = delete;
    ARaiiHelper(ARaiiHelper&&) noexcept = delete;

    ~ARaiiHelper() {
        mCallback();
    }

private:
    std::decay_t<Callback> mCallback;
};