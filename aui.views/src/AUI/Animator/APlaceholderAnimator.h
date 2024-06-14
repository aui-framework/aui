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
// Created by alex2 on 28.11.2020.
//

#pragma once


#include "AAnimator.h"

class API_AUI_VIEWS APlaceholderAnimator: public AAnimator {
public:
    APlaceholderAnimator();


    class ICustomWidth {
    public:

        virtual ~ICustomWidth() = default;
        virtual float getCustomWidthForPlaceholderAnimator() = 0;
    };

protected:
    void doPostRender(AView* view, float theta) override;
};


