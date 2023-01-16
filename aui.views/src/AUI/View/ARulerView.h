// AUI Framework - Declarative UI toolkit for modern C++17
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
// Created by alex2 on 6/25/2021.
//


#pragma once

#include <AUI/View/AView.h>
#include <AUI/Util/ALayoutDirection.h>
#include <AUI/GL/Vao.h>

/**
 * @brief A Photoshop-like ruler for measuring display units.
 * @ingroup userful_views
 */
class API_AUI_VIEWS ARulerView: public AView {
private:
    ALayoutDirection mLayoutDirection;
    _<gl::Vao> mPrecompiledLines;

    int mOffsetPx = 0;

    float mUnit = 1.f;

    int getLongestSide() const;
    int getShortestSide() const;

public:
    explicit ARulerView(ALayoutDirection layoutDirection);

    void render() override;

    void setOffsetPx(int offsetPx) {
        mOffsetPx = offsetPx;
        redraw();
    }

};


