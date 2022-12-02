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

//
// Created by alex2 on 06.12.2020.
//

#pragma once

#include <AUI/Util/ALayoutDirection.h>
#include <AUI/Common/ATimer.h>
#include "AViewContainer.h"
#include "ASpacerExpanding.h"

class AScrollbarButton: public AView {
public:
    AScrollbarButton() {

    }
};
class AScrollbarHandle: public AView {
private:
    int mScrollOffset = 0;
    bool mDragging = false;

public:
    AScrollbarHandle() {

    }

    void onMouseMove(glm::ivec2 pos) override;

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;
};

class AScrollbarOffsetSpacer: public ASpacerExpanding {
public:
    AScrollbarOffsetSpacer(): ASpacerExpanding(0, 0) {

    }

    int getMinimumWidth(ALayoutDirection) override {
        return 0;
    }

    int getMinimumHeight(ALayoutDirection) override {
        return 0;
    }
};

/**
 * @brief A single scrollbar
 * @ingroup useful_views
 * @see AScrollArea
 */
class API_AUI_VIEWS AScrollbar: public AViewContainer {
    friend class AScrollbarHandle;
public:

    explicit AScrollbar(ALayoutDirection direction = ALayoutDirection::VERTICAL);

    [[nodiscard]] int getCurrentScroll() const {
        return mCurrentScroll;
    }

    void setScrollDimensions(size_t viewportSize, size_t fullSize);
    void updateScrollHandleSize();
    void setScroll(int scroll);

    void scroll(int delta) noexcept {
        setScroll(mCurrentScroll + delta);
    }

    void onMouseWheel(const glm::ivec2& pos, const glm::ivec2& delta) override;


protected:
    ALayoutDirection mDirection;
    _<ASpacerExpanding> mOffsetSpacer;
    _<AScrollbarHandle> mHandle;
    _<AScrollbarButton> mForwardButton;
    _<AScrollbarButton> mBackwardButton;
    static _<ATimer> ourScrollButtonTimer;

    size_t mViewportSize = 0, mFullSize = 0;
    int mCurrentScroll = 0;

    void setOffset(size_t o);

    void scrollForward();
    void scrollBackward();

    void handleScrollbar(int s);

    int getMaxScroll();


signals:

    emits<int> scrolled;

    float getAvailableSpaceForSpacer();

    void updateScrollHandleOffset(int max);

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void setSize(glm::ivec2 size) override;
};


