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
#include <variant>
#include "AUI/Traits/concepts.h"
#include <AUI/Reflect/AEnumerate.h>


/**
 * @brief Describes border style, like CSS border-style.
 * @details
 * ABorderStyle is std::variant-based type that stores one of defined subclass types:
 * ```cpp
 * ABorderStyle borderStyle = ABorderStyle::Solid{}; // valid
 * ```
 */
class ABorderStyle {
public:
    struct Solid {};
    
    /**
     * @brief Displays a series of short square-ended dashes or line segments.
     */
    struct Dashed {
        /**
         * @brief If set, renderer would be forced to keep specified space between dashes.
         * @details
         * The value of this property is expressed in border width units. That is, 1.0 equals square.
         *
         * If both dashWidth and spaceBetweenDashes are specified, renderer would roughly cut the line dash sequence.
         * Otherwise, it would try to adjust unset values in order to achieve smooth results.
         */
        AOptional<float> dashWidth;

        /**
         * @brief If set, renderer would be forced to keep specified space between dashes.
         * @details
         * The value of this property is expressed in border width units. That is, 1.0 equals square.
         *
         * If both dashWidth and spaceBetweenDashes are specified, renderer would roughly cut the line dash sequence.
         * Otherwise, it would try to adjust unset values in order to achieve smooth results.
         */
        AOptional<float> spaceBetweenDashes = 1.0f;
    };

    using Impl = std::variant<Solid, Dashed>;

    template<aui::convertible_to<Impl> T>
    ABorderStyle(T&& rhs) noexcept: mValue(std::move(rhs)) {}

    const Impl& value() const noexcept {
        return mValue;
    }

private:
    Impl mValue;
};