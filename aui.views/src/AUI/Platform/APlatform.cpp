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

//
// Created by nelonn on 11/14/25.
//

#include "APlatform.h"

std::unique_ptr<APlatform>& APlatform::currentImpl() {
    static std::unique_ptr<APlatform> impl{};
    return impl;
}

void APlatform::init(std::unique_ptr<APlatform>&& platform) {
    currentImpl() = std::move(platform);
}

APlatform& APlatform::current() {
    auto& impl = currentImpl();
    if (!impl) {
        // TODO: lazy initialize default one
    }
    return *impl;
}
