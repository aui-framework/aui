/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
class AScrollbarHandle: public AView {friend class API_AUI_VIEWS AScrollbar;
private:
    int mScrollOffset = 0;
    bool mDragging = false;

public:
    void setSize(glm::ivec2 size) override;
    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;
    void onPointerPressed(const APointerPressedEvent& event) override;
    void onPointerReleased(const APointerReleasedEvent& event) override;
    void setOverridenSize(int overridenSize) {
        mOverridenSize = overridenSize;
    }

private:
    explicit AScrollbarHandle(AScrollbar& scrollbar) : mScrollbar(scrollbar) {}

    AScrollbar& mScrollbar;
    int mOverridenSize = 0;
};

class AScrollbarOffsetSpacer: public ASpacerExpanding {
public:
    AScrollbarOffsetSpacer(): ASpacerExpanding(0, 0) {

    }

    int getMinimumWidth() override {
        return 0;
    }

    int getMinimumHeight() override {
        return 0;
    }
};

/**
 * @brief A single scrollbar
 *
 * ![](imgs/views/AScrollbar.png)
 *
 * @ingroup views_containment
 * @see AScrollArea
 * @see ASlider
 */
class API_AUI_VIEWS AScrollbar: public AViewContainerBase {
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

    void onScroll(float& delta);

    void onScroll(const AScrollEvent& event) override;

    /**
     * @brief Set stick to end.
     * @param stickToEnd
     * @details
     * When scroll area dimensions is updated (an element added to scroll area) if the scrollbar was scrolled to the end
     * (bottom) the scrollbar automatically scrolls to the ends, keeping the scroll position in place.
     */
    void setStickToEnd(bool stickToEnd) {
        if (stickToEnd) {
            mStickToEnd = StickToEnd{
                .locked = true,
            };
            scrollToEnd();
        } else {
            mStickToEnd.reset();
        }
    }

    void setAppearance(ass::ScrollbarAppearance::AxisValue appearance) {
        mAppearance = appearance;
    }

    void scrollToStart() {
        setScroll(0);
    }

    void scrollToEnd();

    float getAvailableSpaceForSpacer();

    void updateScrollHandleOffset(int max);

    void onPointerPressed(const APointerPressedEvent& event) override;

    void setSize(glm::ivec2 size) override;

    static const _<ATimer>& buttonTimer();


    /**
     * @return max scroll of this scrollbar.
     * @details
     * If viewport size is larger than content size (in the case when contents are smaller than AScrollArea), 0 is
     * returned.
     */
    std::size_t getMaxScroll() const noexcept {
        if (mFullSize <= mViewportSize) {
            return 0;
        }

        return mFullSize - mViewportSize;
    }

signals:

    emits<int> scrolled;

    emits<int> updatedMaxScroll;

    emits<> triggeredManually;

protected:
    ALayoutDirection mDirection;
    _<ASpacerExpanding> mOffsetSpacer;
    _<AScrollbarHandle> mHandle;
    _<AScrollbarButton> mForwardButton;
    _<AScrollbarButton> mBackwardButton;

    size_t mViewportSize = 0, mFullSize = 0;
    int mCurrentScroll = 0;

    void setOffset(size_t o);

    void scrollForward();
    void scrollBackward();

    void handleScrollbar(int s);


private:
    struct StickToEnd {
        /**
         * @brief The stick-to-end behaviour is not overridden by the user.
         */
        bool locked = true;
    };
    /**
     * @brief Stick-to-end behaviour enabled or not.
     * @details
     * Empty = disabled.
     *
     * @see AScrollbar::setStickToEnd()
     */
    AOptional<StickToEnd> mStickToEnd;
    ass::ScrollbarAppearance::AxisValue mAppearance = ass::ScrollbarAppearance::ON_DEMAND;
};


