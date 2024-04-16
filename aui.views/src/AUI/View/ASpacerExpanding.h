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
#include "AView.h"
#include <AUI/Util/Declarative.h>

/**
 * @brief Expanding view which is useful in UI building.
 * @ingroup useful_views
 * @details
 *
 * AExpandingSpacer is expanding blank view which tries acquire space as much as possible.
 */
class API_AUI_VIEWS ASpacerExpanding: public AView
{
public:
	ASpacerExpanding(int w = 4, int h = 4)
	{
		setExpanding({ w, h });
	}
	virtual ~ASpacerExpanding() = default;

    bool consumesClick(const glm::ivec2& pos) override;
};


namespace declarative {
    using SpacerExpanding = aui::ui_building::view<ASpacerExpanding>;
}