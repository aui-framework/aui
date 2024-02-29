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

#include <AUI/View/AViewContainer.h>

class API_AUI_VIEWS ALayoutInflater {
public:
    /**
     * @brief Wraps <code>view</code> with <code>root</code> using <code>Stacked</code> layout and expanding.
     * @details Used when usage of AViewContainer::setContents is not possible (see AViewContainer::setContents)
     *
     * @param root container to wrap with
     * @param view view to be wrapped
     * @note Clears contents of <code>root</code>.
     * @note Stolen from Android.
     * @return <code>root</code>
     */
    static void inflate(aui::no_escape<AViewContainer> root, const _<AView>& view);
};


