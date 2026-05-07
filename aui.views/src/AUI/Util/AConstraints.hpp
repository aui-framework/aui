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
// Created by Nelonn on 5/6/2026.
//

#pragma once

struct AConstraints {
    int minWidth = 0;
    int minHeight = 0;
    int maxWidth = 1000000;
    int maxHeight = 1000000;

    static AConstraints fixedWidth(int width) {
        return AConstraints {
            .minWidth = width,
            .maxWidth = width,
        };
    }

    static AConstraints fixedHeight(int height) {
        return AConstraints {
            .minHeight = height,
            .maxHeight = height,
        };
    }

    constexpr bool isWidthTight() const noexcept {
        return minWidth == maxWidth;
    }

    constexpr bool isHeightTight() const noexcept {
        return minHeight == maxHeight;
    }

    constexpr bool operator==(const AConstraints& other) const noexcept {
        return minWidth == other.minWidth &&
               minHeight == other.minHeight &&
               maxWidth == other.maxWidth &&
               maxHeight == other.maxHeight;
    }
};
