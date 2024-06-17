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

#include "AAbstractLabel.h"

/**
 * @brief Represents a simple single-line text display view.
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/ALabel.png">
 * @ingroup useful_views
 * @details
 * ALabel is used for displayed nonformatted single-line text.
 */
class API_AUI_VIEWS ALabel: public AAbstractLabel {
public:
    using AAbstractLabel::AAbstractLabel;
};


namespace declarative {
    using Label = aui::ui_building::view<ALabel>;
}

template<>
struct ADataBindingDefault<ALabel, AString> {
   public:
    static void setup(const _<AString>& view) {
    }

    static auto getSetter() { return &ALabel::setText; }
};
