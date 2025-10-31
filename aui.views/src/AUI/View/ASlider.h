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


#include "AViewContainer.h"
#include "AProgressBar.h"


namespace declarative {

/**
 * @brief Slider control.
 *
 * ![](imgs/views/ASlider.png)
 *
 * @ingroup views_input
 */
struct API_AUI_VIEWS Slider {
    contract::In<aui::float_within_0_1> value;
    contract::Slot<aui::float_within_0_1> onValueChanged;
    _<AView> track = defaultTrack(value);
    _<AView> handle = defaultHandle();

    _<AView> operator()();

private:
    static _<AView> defaultTrack(const contract::In<aui::float_within_0_1>& value);
    static _<AView> defaultHandle();
};
}

