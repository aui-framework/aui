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

#include <AUI/Util/ALayoutDirection.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include "AViewContainer.h"
#include "AHDividerView.h"
#include "AVDividerView.h"
#include "ASplitterHelper.h"
#include <AUI/View/ASpacerExpanding.h>

/**
 * @brief A resizeable horizontal or vertical layout.
 *
 * ![](imgs/views/ASplitter.png)
 *
 * @ingroup views_containment
 * @details
 * ASplitter lets the user control the size of child views by dragging the boundary between them. Any number of views
 * may be controlled by a single splitter.
 *
 * Generally, ASplitter mimics behaviour of linear layouts (i.e., `Vertical` and `Horizontal`):
 * 1. if there's no expanding view, both linear layouts and ASplitter leave blank space at the end
 * 2. expanding views use all available space.
 *
 * ASplitter is applying geometry via min size-like logic by custom ALayout implementation.
 *
 * ASplitter tends to reclaim space from non-expanding views in favour to expanding views. As such, there should be at
 * least 1 view with expanding. If there's no such view, ASplitter adds ASpacerFixed to the end.
 *
 * Valid expandings should be applied before constructing ASplitter. ASplitter does not support changing expanding on
 * the fly.
 *
 * ASplitter is constructed by builder. Use `ASplitter::Horizontal()` and `ASplitter::Vertical()`.
 */
class API_AUI_VIEWS ASplitter: public AViewContainerBase
{
public:
    virtual ~ASplitter() = default;

    void onPointerPressed(const APointerPressedEvent& event) override;

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;

    void setSize(glm::ivec2 size) override;

    void onPointerReleased(const APointerReleasedEvent& event) override;

    void onClickPrevented() override;

    template<typename Layout>
    class Builder;
    using Vertical = Builder<AVerticalLayout>;
    using Horizontal = Builder<AHorizontalLayout>;

private:
    template<typename Layout>
    friend class Builder;

    ASplitterHelper mHelper;

    ASplitter();

};

template<typename Layout>
class ASplitter::Builder {
    friend class ASplitter;
private:
    AVector<_<AView>> mItems;
    glm::ivec2 mExpanding{};

public:
    Builder& withItems(AVector<_<AView>> items) {
        mItems = std::move(items);
        return *this;
    }

    Builder& withExpanding(glm::ivec2 expanding = { 2, 2 }) {
        mExpanding = expanding;
        return *this;
    }

    API_AUI_VIEWS _<AView> build();

    operator _<AView>() {
        return build();
    }
};
