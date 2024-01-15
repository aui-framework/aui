// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <glm/glm.hpp>
#include <variant>
#include "AUI/Traits/concepts.h"
#include <AUI/Reflect/AEnumerate.h>


/**
 * @brief Describes border style, like CSS border-style.
 * @details
 * ABorderStyle is std::variant-based type that stores one of defined subclass types:
 * @code{cpp}
 * ABorderStyle borderStyle = ABorderStyle::Solid{}; // valid
 * @endcode
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