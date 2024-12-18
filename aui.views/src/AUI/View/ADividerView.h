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

//
// Created by alex2 on 28.09.2020.
//

#pragma once


#include "AView.h"

/**
 * @brief Base class for AHDividerView and AVDivierView.
 */
class API_AUI_VIEWS ADividerView: public AView {
public:
    ADividerView();
    ~ADividerView() override;

    bool consumesClick(const glm::ivec2& pos) override;
};


