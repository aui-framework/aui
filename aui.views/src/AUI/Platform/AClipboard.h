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
// Created by alex2 on 26.11.2020.
//

#pragma once


#include <AUI/Common/AString.h>

/**
 * @brief Access to the system clipboard (text only).
 * @ingroup views
 * @details
 * ```cpp
 * AClipboard::copyToClipboard("Hello");
 * if (!AClipboard::isEmpty()) {
 *     AString s = AClipboard::pasteFromClipboard();
 * }
 * ```
 *
 * Text views such as [ATextField] and [ATextArea] handle ++ctrl+c++, ++ctrl+x++ and ++ctrl+v++ on their own; use
 * AClipboard when you need to put or take text programmatically, i.e., from a "Copy link" button.
 */
class API_AUI_VIEWS AClipboard {
public:
    static void copyToClipboard(const AString& text);
    static bool isEmpty();
    static AString pasteFromClipboard();
};