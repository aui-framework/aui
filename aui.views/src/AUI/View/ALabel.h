// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
