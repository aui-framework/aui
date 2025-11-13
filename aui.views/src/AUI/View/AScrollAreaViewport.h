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
#include "AScrollbar.h"
#include "glm/fwd.hpp"

namespace declarative {
/**
 * <!-- aui:no_dedicated_page -->
 */
struct API_AUI_VIEWS ScrollAreaViewport {
    struct State {
        /**
         * @brief Current scroll offset of the viewport in pixels.
         * @details
         * The value is a 2‑component unsigned vector where `x` represents the horizontal
         * scroll and `y` represents the vertical scroll.  The coordinate system starts
         * at the top‑left corner of the content.  A value of `{0, 0}` means the viewport
         * is scrolled to the origin.
         */
        AProperty<glm::uvec2> scroll = glm::uvec2{0, 0};

        /**
         * @brief Size of the visible area of the viewport in pixels.
         * @details
         * This value is updated by the layout system and represents the width and height
         * of the area that can be seen by the user.  It is used to determine whether
         * scrollbars should be shown and to calculate the maximum scroll offset.
         */
        AProperty<glm::uvec2> viewportSize = glm::uvec2{0, 0};

        /**
         * @brief Full size of the scrollable content.
         * @details
         * Represents the width and height of the entire content area that the
         * viewport scrolls over.  Combined with `viewportSize`, it determines
         * the maximum scroll offset and whether scrolling is required.
         */
        AProperty<glm::uvec2> fullContentSize = glm::uvec2{0, 0};

    };

    /**
     * @brief Holds the state of the viewport, including scroll position.
     */
    _<State> state = _new<State>();

    /**
     * @brief The content view displayed inside the viewport.
     */
    aui::non_null<_<AView>> content;

    _<AView> operator()();

};
}   // namespace declarative

/**
 * @brief Inner component of AScrollArea that manages rendering and event handling with custom offset (scroll).
 * @details
 * This view is intended to store only one single view with setContents()/contents() methods.
 *
 * This view does not handle scroll events and touch events related to scroll. Use [AScrollArea] for such case.
 *
 * ## API surface
 *
 * <!-- aui:steal_documentation declarative::ScrollAreaViewport -->
 */
class API_AUI_VIEWS AScrollAreaViewport : public AViewContainerBase {
public:
    class Inner;

    AScrollAreaViewport(_<declarative::ScrollAreaViewport::State> state = _new<declarative::ScrollAreaViewport::State>());
    ~AScrollAreaViewport() override;

    void setContents(_<AView> content);

    [[nodiscard]]
    const _<AView>& contents() const noexcept {
        return mContents;
    }

    void applyGeometryToChildren() override;

    bool consumesClick(const glm::ivec2& position) override { return true; }

    void setScroll(glm::uvec2 scroll) {
        if (mState->scroll == scroll) [[unlikely]] {
            return;
        }
        mState->scroll = scroll;
    }

    void setScrollX(unsigned scroll) {
        if (mState->scroll->x == scroll) [[unlikely]] {
            return;
        }
        mState->scroll.writeScope()->x = scroll;
    }

    void setScrollY(unsigned scroll) {
        if (mState->scroll->y == scroll) [[unlikely]] {
            return;
        }
        mState->scroll.writeScope()->y = scroll;
    }

    [[nodiscard]]
    auto& scroll() const noexcept {
        return mState->scroll;
    }

    /**
     * @brief Compensates layout updates made in applyLayoutUpdate by scrolling by a diff of relative position of
     * anchor.
     * @param anchor direct or indirect child used as an anchor.
     * @param applyLayoutUpdate layout update procedure.
     * @param diffMask mask that is used to control axes of the compensation. Default is `{1, 1}`.
     * @details
     * Helps preventing visual layout jittering by querying relative to AScrollAreaViewport position of anchor before
     * and after applyLayoutUpdate. The diff of relative position is then used to scroll the viewport, maintaining
     * consistent visual position of anchor.
     *
     * anchor must be direct or indirect child to this AScrollAreaViewport before and after applyLayoutUpdate.
     *
     * The scroll operation made within this method does not prevent scroll animation nor kinetic effects.
     */
    template <aui::invocable ApplyLayoutUpdate>
    void compensateLayoutUpdatesByScroll(_<AView> anchor, ApplyLayoutUpdate&& applyLayoutUpdate, glm::ivec2 diffMask = glm::ivec2(1, 1)) {
        auto queryRelativePosition = [&] {
            glm::ivec2 accumulator {};
            for (auto v = anchor.get(); v != nullptr && v->getParent() != this; v = v->getParent()) {
                accumulator += v->getPosition();
            }
            return accumulator;
        };
        auto before = queryRelativePosition();
        applyLayoutUpdate();
        auto after = queryRelativePosition();
        auto diff = after - before;
        diff *= diffMask;
        mState->scroll = glm::max(glm::ivec2(*mState->scroll) + diff, glm::ivec2(0));
    }

private:
    _<Inner> mInner;
    _<AView> mContents;
    _<declarative::ScrollAreaViewport::State> mState;
};
