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

//
// Created by alex2 on 23.10.2020.
//

#pragma once

#include <AUI/Image/IDrawable.h>
#include <AUI/Util/Declarative.h>
#include "AView.h"

/**
 * @brief Simple view to draw an IDrawable.
 * @ingroup useful_views
 */
class API_AUI_VIEWS ADrawableView: public AView {
private:
    _<IDrawable> mDrawable;

public:
    explicit ADrawableView(const AUrl& url);
    explicit ADrawableView(const _<IDrawable>& drawable);
    void render() override;

    void setDrawable(const _<IDrawable>& drawable) {
        mDrawable = drawable;
        redraw();
    }

protected:

};


namespace declarative {
    using Icon = aui::ui_building::view<ADrawableView>;
}