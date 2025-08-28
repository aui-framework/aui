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


/**
 * @brief Inner component of AScrollArea that manages rendering and event handling with custom offset (scroll).
 * @ingroup views_containment
 * @details
 * This view is intended to store only one single view with setContents()/contents() methods.
 *
 * This view does not handle scroll events and touch events related to scroll. Use AScrollArea for such case.
 */
class API_AUI_VIEWS AScrollAreaViewport: public AViewContainerBase {
public:
    class Inner;

    AScrollAreaViewport();
    ~AScrollAreaViewport() override;

    void setContents(_<AView> content);

    [[nodiscard]]
    const _<AView>& contents() const noexcept {
        return mContents;
    }

    void applyGeometryToChildren() override;

    bool consumesClick(const glm::ivec2& position) override {
        return true;
    }

    void setScroll(glm::uvec2 scroll) {
        if (mScroll == scroll) [[unlikely]] {
            return;
        }
        mScroll = scroll;
        updateContentsScroll();
    }

    void setScrollX(unsigned scroll) {
        if (mScroll.x == scroll) [[unlikely]] {
            return;
        }
        mScroll.x = scroll;
        updateContentsScroll();
    }

    void setScrollY(unsigned scroll) {
        if (mScroll.y == scroll) [[unlikely]] {
            return;
        }
        mScroll.y = scroll;
        updateContentsScroll();
    }

    [[nodiscard]]
    auto scroll() const noexcept {
        return APropertyDef {
            this,
            &AScrollAreaViewport::mScroll,
            &AScrollAreaViewport::setScroll,
            mScrollChanged,
        };
    }

    /**
     * @brief Compensates layout updates made in applyLayoutUpdate by scrolling by a diff of relative position of anchor.
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
    template<aui::invocable ApplyLayoutUpdate>
    void compensateLayoutUpdatesByScroll(_<AView> anchor, ApplyLayoutUpdate&& applyLayoutUpdate, glm::ivec2 diffMask = glm::ivec2(1, 1));

private:
    _<Inner> mInner;
    _<AView> mContents;

    glm::uvec2 mScroll = {0, 0};
    emits<glm::uvec2> mScrollChanged;

    void updateContentsScroll();
};

template <aui::invocable ApplyLayoutUpdate>
void AScrollAreaViewport::compensateLayoutUpdatesByScroll(
    _<AView> anchor, ApplyLayoutUpdate&& applyLayoutUpdate, glm::ivec2 diffMask) {
    auto queryRelativePosition = [&] {
      glm::ivec2 accumulator{};
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
    mScroll = glm::max(glm::ivec2(mScroll) + diff, glm::ivec2(0));
    updateContentsScroll();
}
