/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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

class AScrollAreaContainer;

/**
 * @brief Inner component of AScrollArea that manages rendering and event handling with custom offset (scroll).
 * @ingroup useful_views
 * @details
 * This view is intended to store only one single view with setContents()/contents() methods.
 *
 * This view does not handle scroll events and tocuh events related to scroll. Use AScrollArea for such case.
 */
class API_AUI_VIEWS AScrollAreaViewport: public AViewContainer {
public:
    AScrollAreaViewport();
    ~AScrollAreaViewport() override;

    void setContents(_<AView> content);

    [[nodiscard]]
    const _<AView>& contents() const noexcept {
        return mContents;
    }

    void updateLayout() override;
  
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
    glm::uvec2 scroll() const noexcept {
        return mScroll;
    }

private:
    class Inner;
    _<Inner> mInner;
    _<AView> mContents;

    glm::uvec2 mScroll = {0, 0};

    void updateContentsScroll();
};

