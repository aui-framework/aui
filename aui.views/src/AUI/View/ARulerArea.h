// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
#include "ARulerView.h"

/**
 * @brief Photoshop-like ruler area for measuring display units.
 * @ingroup useful_views
 * @details
 * Consists of vertical and horizontal @ref ARulerView "rulers".
 */
class API_AUI_VIEWS ARulerArea: public AViewContainer {
private:
    _<AView> mWrappedView;
    _<ARulerView> mHorizontalRuler;
    _<ARulerView> mVerticalRuler;
    glm::ivec2 mMousePos;

    void setWrappedViewPosition(const glm::ivec2& pos);

    glm::ivec2 getTargetPosition() const;

protected:
    void updatePosition();

public:
    ARulerArea(const _<AView>& wrappedView);
    void setSize(glm::ivec2 size) override;
    void onMouseMove(glm::ivec2 pos) override;
    void render() override;
};

