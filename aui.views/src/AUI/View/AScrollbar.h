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
 * @details
 * The `AScrollbar` class represents a scroll bar that can be attached to an
 * `AScrollArea`.  It supports both vertical and horizontal orientations.
 *
 * ## API surface
 *
 * <!-- aui:steal_documentation declarative::Scrollbar -->
 */
class API_AUI_VIEWS AScrollbar: public AViewContainerBase {
    friend class AScrollbarHandle;
public:

    /**
     * @brief Constructor
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    explicit AScrollbar(ALayoutDirection direction = ALayoutDirection::VERTICAL);

    /**
     * @brief Get current scroll position
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    [[nodiscard]] int getCurrentScroll() const {
        return mCurrentScroll;
    }

    /**
     * @brief Set scroll dimensions
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    void setScrollDimensions(size_t viewportSize, size_t fullSize);
    /**
     * @brief Update scroll handle size
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    void updateScrollHandleSize();
    /**
     * @brief Set scroll position
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    void setScroll(int scroll);

    /**
     * @brief Scroll by delta
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    void scroll(int delta) noexcept {
        setScroll(mCurrentScroll + delta);
    }

    /**
     * @brief Handle scroll delta
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    void onScroll(float& delta);

    void onScroll(const AScrollEvent& event) override;

    /**
     * @brief Set stick to end.
     * @param stickToEnd
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     *
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

    /**
     * @brief Set scrollbar appearance
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    void setAppearance(ass::ScrollbarAppearance::AxisValue appearance) {
        mAppearance = appearance;
    }

    /**
     * @brief Scroll to start
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    void scrollToStart() {
        setScroll(0);
    }

    /**
     * @brief Scroll to end
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    void scrollToEnd();

    /**
     * @brief Get available space for spacer
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    float getAvailableSpaceForSpacer();

    /**
     * @brief Update scroll handle offset
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    void updateScrollHandleOffset(int max);

    void onPointerPressed(const APointerPressedEvent& event) override;

    void setSize(glm::ivec2 size) override;

    /**
     * @brief Get button timer
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    static const _<ATimer>& buttonTimer();


    /**
     * @return max scroll of this scrollbar.
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     * If viewport size is larger than content size (in the case when contents are smaller than AScrollArea), 0 is
     * returned.
     */
    std::size_t getMaxScroll() const noexcept {
        if (mFullSize <= mViewportSize) {
            return 0;
        }

        return mFullSize - mViewportSize;
    }

    /**
     * @brief Get viewport size
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    [[nodiscard]] size_t viewportSize() const { return mViewportSize; }
    
    /**
     * @brief Get full content size
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    [[nodiscard]] size_t fullSize() const { return mFullSize; }

signals:
    emits<unsigned> scrolled;
    emits<unsigned> scrolledByUser;

    emits<int> updatedMaxScroll;

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

namespace declarative {
/**
 * <!-- aui:no_dedicated_page -->
 */
struct API_AUI_VIEWS Scrollbar {
    /**
     * @brief Scrollbar orientation.
     * @details
     * Determines whether the scrollbar is vertical or horizontal.
     */
    ALayoutDirection direction = ALayoutDirection::VERTICAL;

    /**
     * @brief Current scroll position.
     * @details
     * The value represents the current scroll offset in pixels.
     *
     * Defines scroll handle position.
     */
    contract::In<unsigned> scroll;

    /**
     * @brief Size of the visible viewport.
     * @details
     * Specifies the width/height of the area that can be seen in pixels.
     *
     * Used to calculate scroll handle size.
     */
    contract::In<unsigned> viewportSize;

    /**
     * @brief Total size of the scrollable content.
     * @details
     * Represents the full width/height of the content that can be scrolled
     * through in pixels.
     *
     * Used to calculate scroll handle size.
     */
    contract::In<unsigned> fullContentSize;

    /**
     * @brief Scrollbar appearance.
     * @details
     * Determines when the scrollbar is displayed. The value is of type
     * `ass::ScrollbarAppearance::AxisValue` and can be one of
     * `ALWAYS`, `ON_DEMAND`, or `NEVER`.
     */
    contract::In<ass::ScrollbarAppearance::AxisValue> scrollbarAppearance = ass::ScrollbarAppearance::ON_DEMAND;

    /**
     * @brief Signal emitted when the scroll position changes.
     * @details
     * The signal carries the new scroll offset value. It is intended to be
     * connected to other components that need to react to scrolling.
     */
    contract::Slot<unsigned> onScrollChange;

    _<AView> operator()();
};
}

