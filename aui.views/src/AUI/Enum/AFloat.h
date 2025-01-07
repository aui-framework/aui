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


/**
 * @brief Specifies text floating in text wrapping views, i.e, ATextArea, AText.
 * @ingroup ass_properties
 */
enum class AFloat {
    /**
     * Entry's default position behaviour.
     */
    NONE,

    /**
     * Entry is placed on the row where's it appeared. Entry is snapped to the left border. Other entries are
     * wrapped around this entry.
     */
    LEFT,

    /**
     * Entry is placed on the row where's it appeared. Entry is snapped to the right border. Other entries are
     * wrapped around this entry.
     */
    RIGHT
};