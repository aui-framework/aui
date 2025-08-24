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

#pragma once


#include "AViewContainer.h"
#include "AScrollAreaViewport.h"
#include "AScrollbar.h"
#include "glm/fwd.hpp"


/**
 * @brief A scrollable container with vertical and horizontal scrollbars.
 * @ingroup useful_views
 * @details
 * AScrollArea should have expanding, or fixed size, or max size to be set in order to define it's sizing rules, as it's
 * size cannot be defined by it's contents for obvious reasons.
 *
 * FixedSize and Expanding stylesheet properties would work as expected. If neither of them is set, AScrollArea would
 * occupy size by minimum size of it's contents, as a AViewContainer would do. In such case, you may restrict maximum
 * size of AScrollArea with MaxSize property. AScrollArea will not exceed MaxSize, but also become actual scroll area,
 * involving displaying scroll bars and handling scroll events. This behaviour is similar to Text.
 *
 * Expanding is enabled by default. It can be disabled with ass::Expanding(0) property.
 *
 * Behaviour of vertical and horizontal axes are independent from each other. This behaviour is similar to Text.
 */
class API_AUI_VIEWS AScrollArea: public AViewContainerBase {
public:
    class Builder;

public:
    AScrollArea();

    void setSize(glm::ivec2 size) override;
    void setContents(_<AView> content) {
        mInner->setContents(std::move(content));
    }

    int getContentMinimumWidth() override;
    int getContentMinimumHeight() override;

    void onPointerPressed(const APointerPressedEvent& event) override;
    void onPointerReleased(const APointerReleasedEvent& event) override;

    void setScroll(glm::uvec2 scroll) {
        setScrollX(scroll.x);
        setScrollY(scroll.y);
    }

    void setScrollX(unsigned scroll) {
        AUI_NULLSAFE(mHorizontalScrollbar)->setScroll(int(scroll));
        else mInner->setScrollX(scroll);
    }

    void setScrollY(unsigned scroll) {
        AUI_NULLSAFE(mVerticalScrollbar)->setScroll(int(scroll));
        else mInner->setScrollY(scroll);
    }

    /**
     * @brief Set stick to end.
     * @see AScrollbar::setStickToEnd
     */
    void setStickToEnd(bool stickToEnd) {
        AUI_NULLSAFE(mHorizontalScrollbar)->setStickToEnd(stickToEnd);
        AUI_NULLSAFE(mVerticalScrollbar)->setStickToEnd(stickToEnd);
    }

    void scroll(glm::ivec2 by) noexcept {
        scroll(by.x, by.y);
    }

    void scroll(int deltaByX, int deltaByY) noexcept {
        AUI_NULLSAFE(mHorizontalScrollbar)->scroll(deltaByX);
        else mInner->setScrollX(mInner->scroll()->x + deltaByX);
        AUI_NULLSAFE(mVerticalScrollbar)->scroll(deltaByY);
        else mInner->setScrollY(mInner->scroll()->y + deltaByY);
    }

    bool onGesture(const glm::ivec2 &origin, const AGestureEvent &event) override;

    void onScroll(const AScrollEvent& event) override;

    void setScrollbarAppearance(ass::ScrollbarAppearance scrollbarAppearance) {
        AUI_NULLSAFE(mHorizontalScrollbar)->setAppearance(scrollbarAppearance.getHorizontal());
        AUI_NULLSAFE(mVerticalScrollbar)->setAppearance(scrollbarAppearance.getVertical());
    }

    /**
     * @brief Scrolls to the specified target view.
     * @param target target view to scroll to. Must be direct or indirect child.
     * @param nearestBorder if true, the scroll is performed up to the nearest border of scroll area, and if the target
     *        is already fully visible, then scrollTo does not take effect. If false, the scroll is performed up to the
     *        top border of the target view.
     */
    void scrollTo(const _<AView>& target, bool nearestBorder = true) {
        if (!target) {
            // nullptr target???
            return;
        }
        scrollTo(ARect<int>::fromTopLeftPositionAndSize(target->getPositionInWindow(), target->getSize()), nearestBorder);
    }

    /**
     * @brief Scrolls to the specified rectangle area.
     * @param target target rectangle area in coordinate space of the window.
     * @param nearestBorder if true, the scroll is performed up to the nearest border of scroll area, and if the target
     *        is already fully visible, then scrollTo does not take effect. If false, the scroll is performed up to the
     *        top border of the target view.
     */
    void scrollTo(ARect<int> target, bool nearestBorder = true);

    /**
     * @see mIsWheelScrollable
     */
    void setWheelScrollable(bool value) {
        mIsWheelScrollable = value;
    }

    class Builder {
    friend class AScrollArea;
    private:
        _<AScrollbar> mExternalVerticalScrollbar;
        _<AScrollbar> mExternalHorizontalScrollbar;
        _<AView> mContents;
        bool mExpanding = false;

    public:
        Builder() = default;

        Builder& withExternalVerticalScrollbar(_<AScrollbar> externalVerticalScrollbar) {
            mExternalVerticalScrollbar = std::move(externalVerticalScrollbar);
            return *this;
        }

        Builder& withExternalHorizontalScrollbar(_<AScrollbar> externalHorizontalScrollbar) {
            mExternalHorizontalScrollbar = std::move(externalHorizontalScrollbar);
            return *this;
        }

        Builder& withContents(_<AView> contents) {
            mContents = std::move(contents);
            return *this;
        }

        Builder& withExpanding() {
            mExpanding = true;
            return *this;
        }

        _<AScrollArea> build() {
            return aui::ptr::manage_shared(new AScrollArea(*this));
        }

        operator _<AView>() {
            return build();
        }
        operator _<AViewContainerBase>() {
            return build();
        }
    };

    [[nodiscard]]
    const _<AView>& contents() const noexcept {
        return mInner->contents();
    }

    [[nodiscard]]
    const _<AScrollbar>& verticalScrollbar() const noexcept {
        return mVerticalScrollbar;
    }

    [[nodiscard]]
    const _<AScrollbar>& horizontalScrollbar() const noexcept {
        return mHorizontalScrollbar;
    }
    
    [[nodiscard]]
    glm::uvec2 scroll() const noexcept {
        return mInner->scroll();
    }
protected:
    explicit AScrollArea(const Builder& builder);

private:
    _<AScrollAreaViewport> mInner;
    _<AScrollbar> mVerticalScrollbar;
    _<AScrollbar> mHorizontalScrollbar;

    /**
     * @brief Determines whether AScrollArea can be scrolled with mouse wheel or can be scrolled with touch only.
     */
    bool mIsWheelScrollable = true;
};

