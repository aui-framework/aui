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
// Created by alex2 on 06.12.2020.
//

#pragma once

#include <AUI/Util/ALayoutDirection.h>
#include <AUI/Common/ATimer.h>
#include "AViewContainer.h"
#include "ASpacerExpanding.h"

class API_AUI_VIEWS AScrollbar;

class AScrollbarButton: public AView {
public:
    AScrollbarButton() {

    }
};
class AScrollbarHandle: public AView {
friend class API_AUI_VIEWS AScrollbar;
private:
    int mScrollOffset = 0;
    bool mDragging = false;

public:
    void setSize(glm::ivec2 size) override;
    void onMouseMove(glm::ivec2 pos) override;
    void onPointerPressed(const APointerPressedEvent& event) override;
    void onPointerReleased(const APointerReleasedEvent& event) override;
    void setOverridenSize(int overridenSize) {
        mOverridenSize = overridenSize;
    }

private:
    explicit AScrollbarHandle(AScrollbar& scrollbar) : mScrollbar(scrollbar) {}

    AScrollbar& mScrollbar;
    int mOverridenSize;
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

    void onScroll(const AScrollEvent& event) override;

    /**
     * @brief Set stick to end.
     * @param stickToEnd
     * @details
     * When scroll area dimensions is updated (an element added to scroll area) if the scrollbar was scrolled to the end
     * (bottom) the scrollbar automatically scrolls to the ends, keeping the scroll position in place.
     */
    void setStickToEnd(bool stickToEnd) {
        mStickToEnd = stickToEnd;
        if (stickToEnd) {
            scrollToEnd();
        }
    }

    void setAppearance(ScrollbarAppearance::AxisValue appearance) {
        mAppearance = appearance;
    }

    void scrollToStart() {
        setScroll(0);
    }

    void scrollToEnd() {
        setScroll(getMaxScroll());
    }

signals:

    emits<int> scrolled;

    float getAvailableSpaceForSpacer();

    void updateScrollHandleOffset(int max);

    void onPointerPressed(const APointerPressedEvent& event) override;

    void setSize(glm::ivec2 size) override;

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

    std::size_t getMaxScroll() const noexcept {
        if (mFullSize <= mViewportSize) {
            return 0;
        }

        return mFullSize - mViewportSize;
    }

private:
    bool mStickToEnd = false;
    ScrollbarAppearance::AxisValue mAppearance = ScrollbarAppearance::INVISIBLE;
};


