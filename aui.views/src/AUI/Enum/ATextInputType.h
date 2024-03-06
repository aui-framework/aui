//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <AUI/Reflect/AEnumerate.h>

/**
 * @brief Controls IME text input type of the text field.
 * @details
 * The type of information for which to optimize the text input control.
 *
 * On Android, behavior may vary across device and keyboard provider.
 */
enum class ATextInputType {
    /**
     * @brief Optimize for textual information.
     */
    DEFAULT,

    /**
     * @brief Optimize for email addresses.
     */
    EMAIL,

    /**
     * @brief Optimize for multiline textual information.
     */
    MULTILINE,

    /**
     * @brief Optimize for unsigned numerical information without a decimal point.
     */
    NUMBER,

    /**
     * @brief Optimize for URLs.
     */
    URL,
};

AUI_ENUM_VALUES(ATextInputType,
                ATextInputType::DEFAULT,
                ATextInputType::EMAIL,
                ATextInputType::MULTILINE,
                ATextInputType::NUMBER,
                ATextInputType::URL)

