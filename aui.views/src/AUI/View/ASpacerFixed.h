// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#include <AUI/View/AView.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Util/Declarative.h>

/**
 * @brief Fixed-size view which is useful in UI building.
 * @ingroup useful_views
 * @details
 *
 * ABlankSpace is fixed size blank view which acquires specified space.
 */
class API_AUI_VIEWS ASpacerFixed: public AView {
public:
    ASpacerFixed(AMetric space): mSpace(space) {}

    int getContentMinimumWidth(ALayoutDirection layout) override;

    int getContentMinimumHeight(ALayoutDirection layout) override;

private:
    AMetric mSpace;
};


namespace declarative {
    using SpacerFixed = aui::ui_building::view<ASpacerFixed>;
}