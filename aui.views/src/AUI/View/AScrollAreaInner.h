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


#include "AViewContainer.h"
#include "AScrollbar.h"
#include "glm/fwd.hpp"

class AScrollAreaContainer;

/**
 * @brief Inner component of AScrollArea that manages rendering and event handling with custom offset (scroll).
 * @ingroup useful_views
 * @details
 * This view is intended to store only one single view with setContents()/contents() methods.
 *
 * This view does not handle scroll events and tocuh events related to scroll. Use AScrollArea for such case.
 */
class API_AUI_VIEWS AScrollAreaInner: public AViewContainer {
public:
    AScrollAreaInner() = default;

    void setContents(_<AView> content);

    [[nodiscard]]
    const _<AView>& contents() const noexcept {
        return mContents;
    }

    void updateLayout() override;
  
    void setScroll(glm::uvec2 scroll) {
        mScroll = scroll;
        updateContentsScroll();
    }

    void setScrollX(unsigned scroll) {
        mScroll.x = scroll;
        updateContentsScroll();
    }

    void setScrollY(unsigned scroll) {
        mScroll.y = scroll;
        updateContentsScroll();
    }

    [[nodiscard]]
    glm::uvec2 scroll() const noexcept {
        return mScroll;
    }

private:
    _<AView> mContents;
    glm::uvec2 mScroll = {0, 0};

    void updateContentsScroll();
};

