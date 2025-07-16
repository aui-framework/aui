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

#include "AAbstractLabel.h"
#include <AUI/Util/Declarative/Contracts.h>

/**
 * @brief Represents a simple single-line text display view.
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/ALabel.png">
 * @ingroup useful_views
 * @details
 * ALabel is used for displaying nonformatted single-line text.
 *
 * See @ref declarative::Label for a declarative form and examples.
 */
class API_AUI_VIEWS ALabel: public AAbstractLabel {
public:
    using AAbstractLabel::AAbstractLabel;
};


namespace declarative {
/**
 * @declarativeformof{ALabel}
 */
struct Label {
   AOptional<contract::In<AString>> text;

   _<ALabel> operator()() {
       auto label = _new<ALabel>();
       AUI_NULLSAFE(text)->bindTo(label->text());
       return label;
   }
};

/**
 * @declarativeformof{ALabel}
 */
struct Text {
    AOptional<contract::In<AString>> text;

    _<ALabel> operator()() {
        auto label = _new<ALabel>();
        AUI_NULLSAFE(text)->bindTo(label->text());
        return label;
    }
};
}

template<>
struct ADataBindingDefault<ALabel, AString> {
public:
    static auto property(const _<ALabel>& view) {
        return view->text();
    }

    static void setup(const _<ALabel>& view) {
    }

    static auto getSetter() { return &ALabel::setText; }
};
