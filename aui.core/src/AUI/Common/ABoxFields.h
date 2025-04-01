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

#include <glm/glm.hpp>

/**
 * @brief Represents a rectangle fields. Useful for margin and padding around AViews.
 * @ingroup core
 */
struct ABoxFields
{
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;


    [[nodiscard]]
    glm::ivec2 leftTop() const noexcept {
        return { left, top };
    }

    [[nodiscard]]
    glm::ivec2 rightTop() const noexcept {
        return { right, top };
    }

    [[nodiscard]]
    glm::ivec2 leftBottom() const noexcept {
        return { left, bottom };
    }

    [[nodiscard]]
    glm::ivec2 rightBottom() const noexcept {
        return { right, bottom };
    }

    [[nodiscard]]
    int horizontal() const noexcept
    {
        return left + right;
    }

    [[nodiscard]]
    int vertical() const noexcept
    {
        return top + bottom;
    }
    
    [[nodiscard]]
    glm::ivec2 occupiedSize() const noexcept {
        return { horizontal(), vertical() };
    }

};
