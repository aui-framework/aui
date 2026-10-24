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
// Created by alex2 on 7/2/2021.
//


#pragma once


#include <AUI/View/AView.h>

class API_AUI_VIEWS AViewProfiler {
public:
    static void displayBoundsOn(AView& v, ARenderContext ctx);

    /**
     * @brief Draws overflow (clipping) indicators over the view hierarchy of v.
     * @param v root view to walk through, typically ASurface.
     * @param ctx render context of v.
     * @details
     * Every view with AOverflow other than AOverflow::VISIBLE is outlined. If the contents of such view stick out of
     * its bounds (and thus are cut off), the edges where it happens are marked with a band and the amount of the cut
     * off contents in px.
     *
     * Enabled by ASurface::Profiling::showOverflow (devtools).
     */
    static void displayOverflowOn(AView& v, ARenderContext ctx);
};


