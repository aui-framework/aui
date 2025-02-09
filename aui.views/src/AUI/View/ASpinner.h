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


#include "AView.h"

/**
 * @brief Circular infinite progress indicator.
 * @ingroup useful_views
 * @details
 * Rotates its contents as it renders.
 *
 * @note
 * ASpinner constantly requests redraws to maintain its animation which might lead to high performance usage. See
 * ASpinnerV2.
 */
class API_AUI_VIEWS ASpinner: public AView {
public:
    void render(ARenderContext ctx) override;
};


