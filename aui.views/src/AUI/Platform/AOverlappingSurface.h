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

class ABaseWindow;

class API_AUI_VIEWS AOverlappingSurface: public AViewContainer {
    friend class ABaseWindow;
private:
    ABaseWindow* mParentWindow;
    bool mCloseOnClick;

public:
    AOverlappingSurface() = default;

    virtual ~AOverlappingSurface() = default;
    void close();

    bool isCloseOnClick() const {
        return mCloseOnClick;
    }

    void setCloseOnClick(bool closeOnClick) {
        mCloseOnClick = closeOnClick;
    }

    ABaseWindow* getParentWindow() const {
        return mParentWindow;
    }

    virtual void setOverlappingSurfacePosition(glm::ivec2 position) = 0;
    virtual void setOverlappingSurfaceSize(glm::ivec2 size) = 0;
};