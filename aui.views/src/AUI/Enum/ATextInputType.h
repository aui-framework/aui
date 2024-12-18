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

#include <AUI/Reflect/AEnumerate.h>

/**
 * @brief Controls IME text input type of the text field.
 * @ingroup views
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

