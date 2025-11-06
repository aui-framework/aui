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


namespace declarative {

/**
 * <!-- aui:no_dedicated_page -->
 */
struct API_AUI_VIEWS ScrollArea {

    /**
     * @brief A struct passed to scrollbar factory.
     */
    struct ScrollbarInitParams {
        /**
         * @brief Scrollbar orientation.
         */
        ALayoutDirection direction;

        /**
         * @brief Current scroll position of the scrollbar.
         */
        contract::In<unsigned> scroll;

        /**
         * @brief Size of the viewport in the scrollbar's orientation.
         */
        contract::In<unsigned> viewportSize;

        /**
         * @brief Total size of the content in the scrollbar's orientation.
         */
        contract::In<unsigned> fullContentSize;

        /**
         * @brief Callback invoked when the scrollbar's scroll value changes.
         */
        contract::Slot<unsigned> onScrollChange;
    };

    /**
     * @brief Function type that creates a new scrollbar based on ScrollbarInitParams.
     */
    using ScrollbarFactory = _<AView>(ScrollbarInitParams params);

    /**
    * @brief Constructs parameters for a vertical scrollbar based on the current viewport state.
    * @return ScrollbarInitParams configured for vertical scrolling.
    * @details
    * This function is used to generate the initialization parameters required to create a vertical scrollbar
    * that is synchronized with the current state of the scroll area viewport. The result of this function is passed
    * to `verticalScrollbar` factory to create the actual scrollbar instance. Hence, it can be used to create external
    * scrollbars that are linked to the scroll area.
    */
    static ScrollbarInitParams getVerticalScrollbarParams(const _<declarative::ScrollAreaViewport::State>& state);

    /**
     * @brief Constructs parameters for a horizontal scrollbar based on the current viewport state.
     * @return ScrollbarInitParams configured for horizontal scrolling.
     * @details
     * This function is used to generate the initialization parameters required to create a horizontal scrollbar
     * that is synchronized with the current state of the scroll area viewport. The result of this function is passed
     * to `horizontalScrollbar` factory to create the actual scrollbar instance. Hence, it can be used to create
     * external scrollbars that are linked to the scroll area.
     */
    static ScrollbarInitParams getHorizontalScrollbarParams(const _<declarative::ScrollAreaViewport::State>& state);

    /**
     * @brief Current viewport state used by the scroll area.
     * @details
     * This state object holds the scroll position, viewport size, and full content size of the scroll area. It can be
     * used to control the scroll area programmatically or to link external scrollbars to the scroll area.
     */
    _<declarative::ScrollAreaViewport::State> state = _new<declarative::ScrollAreaViewport::State>();

    /**
     * @brief Factory for creating the vertical scrollbar. Defaults to `defaultScrollbar`.
     * @details
     * This factory function is called to create the vertical scrollbar for the scroll area. It can be used to customize
     * the appearance and behavior of the vertical scrollbar, up to completely replacing it with a custom
     * implementation.
     *
     * If set to `nullptr`, no vertical scrollbar will be created. The user can still add an external scrollbar and
     * control the scroll area through the `state` property. See `getVerticalScrollbarParams`.
     */
    std::function<ScrollbarFactory> verticalScrollbar = defaultScrollbar;

    /**
     * @brief Factory for creating the horizontal scrollbar. Defaults to `defaultScrollbar`.
     * @details
     * This factory function is called to create the horizontal scrollbar for the scroll area. It can be used to
     * customize the appearance and behavior of the horizontal scrollbar, up to completely replacing it with a custom
     * implementation.
     *
     * If set to `nullptr`, no horizontal scrollbar will be created. The user can still add an external scrollbar and
     * control the scroll area through the `state` property. See `getHorizontalScrollbarParams`.
     */
    std::function<ScrollbarFactory> horizontalScrollbar = defaultScrollbar;

    /**
     * @brief Default scrollbar factory function.
     * @param params Initialization parameters for the scrollbar (see `getVerticalScrollbarParams` and `getHorizontalScrollbarParams`).
     * @return A new instance of AScrollbar configured with the provided parameters.
     * @details
     * <!-- aui:snippet aui.views/src/AUI/View/AScrollArea.h defaultScrollbar -->
     */
    /// [defaultScrollbar]
    static _<AView> defaultScrollbar(ScrollbarInitParams params) {
        return Scrollbar {
            .direction = std::move(params.direction),
            .scroll = std::move(params.scroll),
            .viewportSize = std::move(params.viewportSize),
            .fullContentSize = std::move(params.fullContentSize),
            .onScrollChange = std::move(params.onScrollChange),
        };
    }
    /// [defaultScrollbar]

    /**
     * @brief Determines whether AScrollArea can be scrolled with mouse wheel or can be scrolled with touch only.
     */
    contract::In<bool> wheelScrollable = true;

    /**
     * @brief Content view displayed inside the scroll area.
     */
    _<AView> content;

    _<AView> operator()();
};
}



/**
 * @brief A scrollable container with vertical and horizontal scrollbars.
 * @ingroup views_containment
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
 *
 * ## API surface
 *
 * <!-- aui:steal_documentation declarative::ScrollArea -->
 */
class API_AUI_VIEWS AScrollArea: public AViewContainerBase {
public:
    AScrollArea();
    explicit AScrollArea(declarative::ScrollArea&& builder);

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


    /**
     * @brief Set scrollbar appearance
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     *
     * Sets the appearance mode for both vertical and horizontal scrollbars.
     */
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
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
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
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    void scrollTo(ARect<int> target, bool nearestBorder = true);

    /**
     * @see mIsWheelScrollable
     */
    void setWheelScrollable(bool value) {
        mIsWheelScrollable = value;
    }

    /**
     * @brief Builder for AScrollArea.
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    class Builder {
    friend class AScrollArea;
    private:
        _<declarative::ScrollAreaViewport::State> mState = _new<declarative::ScrollAreaViewport::State>();
        _<AScrollbar> mExternalVerticalScrollbar;
        _<AScrollbar> mExternalHorizontalScrollbar;
        _<AView> mContents;
        bool mExpanding = false;

    public:
        explicit Builder(_<declarative::ScrollAreaViewport::State> state = _new<declarative::ScrollAreaViewport::State>()): mState(std::move(state)) {}

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

    /**
     * @brief Get contents
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    [[nodiscard]]
    const _<AView>& contents() const noexcept {
        return mInner->contents();
    }

    /**
     * @brief Get vertical scrollbar
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
    [[nodiscard]]
    const _<AScrollbar>& verticalScrollbar() const noexcept {
        return mVerticalScrollbar;
    }

    /**
     * @brief Get horizontal scrollbar
     * @details
     * !!! failure "Deprecated"
     *
     *     Left for compatibility with [retained UI](retained_immediate_ui.md).
     */
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

    // legacy fields. on declarative-based constructor, these fields are null
    _<AScrollbar> mVerticalScrollbar;
    _<AScrollbar> mHorizontalScrollbar;

    // if declarative-based constructor is used, the scroll is updated thru this field.
    _<declarative::ScrollAreaViewport::State> mState;

    /**
     * @brief Determines whether AScrollArea can be scrolled with mouse wheel or can be scrolled with touch only.
     */
    bool mIsWheelScrollable = true;
};
