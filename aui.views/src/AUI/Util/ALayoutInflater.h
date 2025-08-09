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

#include <AUI/View/AViewContainer.h>

class API_AUI_VIEWS ALayoutInflater {
public:
    /**
     * @brief Wraps <code>view</code> with <code>root</code> using <code>Stacked</code> layout and expanding.
     * @details Used when usage of AViewContainer::setContents is not possible (see AViewContainer::setContents)
     * @param root container to wrap with
     * @param view view to be wrapped
     * @details
     * Clears contents of <code>root</code>.
     *
     * Stolen from Android.
     */
    static void inflate(aui::no_escape<AViewContainer> root, const _<AView>& view);
};


