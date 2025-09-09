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
// Created by alex2 on 28.09.2020.
//

#pragma once

#include "ADividerView.h"

/**
 * @brief A horizontal line.
 *
 * ![](imgs/views/AHDividerView.png)
 *
 * @ingroup views_arrangement
 */
class API_AUI_VIEWS AHDividerView: public ADividerView {
public:
    AHDividerView();

    ~AHDividerView() override;
};


