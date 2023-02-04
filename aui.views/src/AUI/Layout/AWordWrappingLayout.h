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


#include <AUI/Util/AViewEntry.h>
#include "ALayout.h"

/**
 * @brief Imitates behaviour of word wrapping, but uses @ref AView "views" instead words
 * @ingroup layout_managers
 */
class API_AUI_VIEWS AWordWrappingLayout: public ALayout {
private:
    AVector<AViewEntry> mViewEntry;

public:
    void onResize(int x, int y, int width, int height) override;

    int getMinimumWidth() override;

    int getMinimumHeight() override;

    void addView(size_t index, const _<AView>& view) override;

    void removeView(size_t index, const _<AView>& view) override;
};


