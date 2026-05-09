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
// Created by alex2772 on 12/6/20.
//

#include "AScrollArea.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/View/AScrollAreaViewport.h"
#include "AUI/View/AView.h"
#include "AUI/View/AViewContainer.h"
#include "glm/fwd.hpp"
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <algorithm>

namespace {
constexpr int UNBOUNDED_CONSTRAINT = 1000000;
}

AScrollArea::AScrollArea() : AScrollArea(Builder {}) { addAssName("AScrollArea"); }

AScrollArea::AScrollArea(const AScrollArea::Builder& builder) {
    addAssName("AScrollArea");

    addView(mInner = _new<AScrollAreaViewport>());
    if (!builder.mExternalVerticalScrollbar) {
        addView(mVerticalScrollbar = _new<AScrollbar>(ALayoutDirection::VERTICAL));
    } else {
        mVerticalScrollbar = builder.mExternalVerticalScrollbar;
    }

    if (!builder.mExternalHorizontalScrollbar) {
        addView(mHorizontalScrollbar = _new<AScrollbar>(ALayoutDirection::HORIZONTAL));
    } else {
        mHorizontalScrollbar = builder.mExternalHorizontalScrollbar;
    }

    if (builder.mContents) {
        setContents(builder.mContents);
    }

    setExpanding();

    connect(mVerticalScrollbar->scrolled, AUI_SLOT(mInner)::setScrollY);
    connect(mHorizontalScrollbar->scrolled, AUI_SLOT(mInner)::setScrollX);
    connect(mInner->scroll().changed, [&](glm::ivec2 scroll) {
        mHorizontalScrollbar->setScroll(scroll.x);
        mVerticalScrollbar->setScroll(scroll.y);
    });
}

bool AScrollArea::hasInternalVerticalScrollbar() const noexcept {
    return mVerticalScrollbar && mVerticalScrollbar->getParent() == this;
}

bool AScrollArea::hasInternalHorizontalScrollbar() const noexcept {
    return mHorizontalScrollbar && mHorizontalScrollbar->getParent() == this;
}

int AScrollArea::measureVerticalScrollbarWidth(int availableHeight) const {
    if (!hasInternalVerticalScrollbar()) {
        return 0;
    }
    if (availableHeight == -1) {
        return mVerticalScrollbar->measure(AConstraints {}).x;
    }
    return mVerticalScrollbar->measure(AConstraints::fixedHeight(std::max(0, availableHeight))).x;
}

int AScrollArea::measureHorizontalScrollbarHeight(int availableWidth) const {
    if (!hasInternalHorizontalScrollbar()) {
        return 0;
    }
    if (availableWidth == -1) {
        return mHorizontalScrollbar->measure(AConstraints {}).y;
    }
    return mHorizontalScrollbar->measure(AConstraints::fixedWidth(std::max(0, availableWidth))).y;
}

AScrollArea::LayoutGeometry AScrollArea::calculateLayout(glm::ivec2 availableSize, bool widthBounded, bool heightBounded) const {
    LayoutGeometry result;
    availableSize = glm::max(availableSize, glm::ivec2(0));

    if (!contents()) {
        result.viewportSize = availableSize;
        result.outerSize = availableSize;
        return result;
    }

    const auto margins = contents()->getMargin().occupiedSize();
    const auto contentMinMax = contents()->computeMinMaxSizes();
    const int naturalContentWidth = margins.x + contentMinMax.max.x;
    const int naturalContentHeight = margins.y + contentMinMax.max.y;

    AConstraints minConstraints;
    minConstraints.minWidth = 0;
    minConstraints.maxWidth = 0; // probe minimum width under impossible width constraint
    minConstraints.minHeight = 0;
    minConstraints.maxHeight = UNBOUNDED_CONSTRAINT; // equivalent of "height is not important"

    const int measuredMinimumContentWidth = margins.x + contents()->measure(minConstraints).x;
    const int minimumScrollableContentWidth =
        std::max(measuredMinimumContentWidth, margins.x + contentMinMax.min.x);

    int viewportWidth = widthBounded
        ? availableSize.x
        : (availableSize.x > 0 ? availableSize.x : std::max(minimumScrollableContentWidth, naturalContentWidth));
    viewportWidth = std::max(viewportWidth, 0);

    auto contentHeightForViewportWidth = [&](int width) {
        const int contentWidth = std::max(0, width - margins.x);
        return contents()->measure(AConstraints::fixedWidth(contentWidth)).y + margins.y;
    };

    int viewportHeight = heightBounded
        ? availableSize.y
        : (availableSize.y > 0 ? availableSize.y : contentHeightForViewportWidth(viewportWidth));
    viewportHeight = std::max(viewportHeight, 0);

    result.verticalScrollbarWidth = measureVerticalScrollbarWidth(viewportHeight);
    result.horizontalScrollbarHeight = measureHorizontalScrollbarHeight(viewportWidth);

    for (int i = 0; i < 3; ++i) {
        const int effectiveViewportWidth =
            std::max(0, viewportWidth - (result.hasVerticalScrollbar ? result.verticalScrollbarWidth : 0));
        const int effectiveViewportHeight =
            std::max(0, viewportHeight - (result.hasHorizontalScrollbar ? result.horizontalScrollbarHeight : 0));

        const bool widthCompressionChangesLayout =
            contentHeightForViewportWidth(effectiveViewportWidth) > naturalContentHeight;

        int contentSurfaceWidth = std::max(effectiveViewportWidth, minimumScrollableContentWidth);
        if (!widthCompressionChangesLayout) {
            contentSurfaceWidth = std::max(contentSurfaceWidth, naturalContentWidth);
        }
        const int contentSurfaceHeight = std::max(
            effectiveViewportHeight,
            contentHeightForViewportWidth(contentSurfaceWidth));

        const bool nextHorizontalScrollbar = contentSurfaceWidth > effectiveViewportWidth;
        const bool nextVerticalScrollbar = contentSurfaceHeight > effectiveViewportHeight;

        result.viewportSize = { effectiveViewportWidth, effectiveViewportHeight };
        result.contentSize = { contentSurfaceWidth, contentSurfaceHeight };

        if (result.hasHorizontalScrollbar == nextHorizontalScrollbar &&
            result.hasVerticalScrollbar == nextVerticalScrollbar) {
            break;
        }

        result.hasHorizontalScrollbar = nextHorizontalScrollbar;
        result.hasVerticalScrollbar = nextVerticalScrollbar;
    }

    result.viewportSize = {
        std::max(0, viewportWidth - (result.hasVerticalScrollbar ? result.verticalScrollbarWidth : 0)),
        std::max(0, viewportHeight - (result.hasHorizontalScrollbar ? result.horizontalScrollbarHeight : 0)),
    };
    const bool widthCompressionChangesLayout =
        contentHeightForViewportWidth(result.viewportSize.x) > naturalContentHeight;
    int contentSurfaceWidth = std::max(result.viewportSize.x, minimumScrollableContentWidth);
    if (!widthCompressionChangesLayout) {
        contentSurfaceWidth = std::max(contentSurfaceWidth, naturalContentWidth);
    }
    result.contentSize = {
        contentSurfaceWidth,
        std::max(result.viewportSize.y, contentHeightForViewportWidth(contentSurfaceWidth)),
    };
    result.outerSize = {
        result.viewportSize.x + (result.hasVerticalScrollbar ? result.verticalScrollbarWidth : 0),
        result.viewportSize.y + (result.hasHorizontalScrollbar ? result.horizontalScrollbarHeight : 0),
    };
    return result;
}

glm::ivec2 AScrollArea::onIntrinsicMeasure(AConstraints constraints) {
    const bool widthBounded = constraints.maxWidth < UNBOUNDED_CONSTRAINT;
    const bool heightBounded = constraints.maxHeight < UNBOUNDED_CONSTRAINT;

    auto layout = calculateLayout(
        {
            widthBounded ? constraints.maxWidth : 0,
            heightBounded ? constraints.maxHeight : 0,
        },
        widthBounded,
        heightBounded);

    return {
        std::clamp(layout.outerSize.x, constraints.minWidth, constraints.maxWidth),
        std::clamp(layout.outerSize.y, constraints.minHeight, constraints.maxHeight),
    };
}

AMinMaxSizes AScrollArea::onComputeIntrinsicMinMaxSizes(int height) {
    if (!contents()) {
        return {};
    }
    const auto contentMinMax = contents()->computeMinMaxSizes();
    const auto minOuterSize = calculateLayout(
        {
            contentMinMax.min.x,
            height == -1 ? 0 : height,
        },
        false,
        height != -1).outerSize;
    const auto maxOuterSize = calculateLayout(
        {
            contentMinMax.max.x,
            height == -1 ? 0 : height,
        },
        false,
        height != -1).outerSize;
    return {
        .min = minOuterSize,
        .max = maxOuterSize,
    };
}

void AScrollArea::applyGeometryToChildren() {
    const glm::ivec2 paddedPosition = { mPadding.left, mPadding.top };
    const glm::ivec2 paddedSize = glm::max(getSize() - mPadding.occupiedSize(), glm::ivec2(0));

    auto layout = calculateLayout(paddedSize, true, true);

    mInner->setScrollSurfaceSize(layout.contentSize);
    mInner->layout(paddedPosition, layout.viewportSize);

    AUI_NULLSAFE(mVerticalScrollbar)->setScrollDimensions(layout.viewportSize.y, layout.contentSize.y);
    AUI_NULLSAFE(mHorizontalScrollbar)->setScrollDimensions(layout.viewportSize.x, layout.contentSize.x);

    if (hasInternalVerticalScrollbar()) {
        if (layout.hasVerticalScrollbar) {
            mVerticalScrollbar->setVisibility(Visibility::VISIBLE);
            mVerticalScrollbar->layout(
                paddedPosition.x + layout.viewportSize.x,
                paddedPosition.y,
                layout.verticalScrollbarWidth,
                layout.viewportSize.y);
        } else {
            mVerticalScrollbar->setVisibility(Visibility::GONE);
        }
    }

    if (hasInternalHorizontalScrollbar()) {
        if (layout.hasHorizontalScrollbar) {
            mHorizontalScrollbar->setVisibility(Visibility::VISIBLE);
            mHorizontalScrollbar->layout(
                paddedPosition.x,
                paddedPosition.y + layout.viewportSize.y,
                layout.viewportSize.x,
                layout.horizontalScrollbarHeight);
        } else {
            mHorizontalScrollbar->setVisibility(Visibility::GONE);
        }
    }
}

void AScrollArea::setSize(glm::ivec2 size) {
    AViewContainerBase::setSize(size);
}

void AScrollArea::onScroll(const AScrollEvent& event) {
    AViewContainerBase::onScroll(event);
    if (!mIsWheelScrollable) {
        return;
    }

    // Checking visibility here for both scrollbars:
    // The logic behind this check is user would not probably intend scroll a scroll area without visible scrollbars.
    // Scroll bar visibility is determined primarily by ass::ScrollbarAppearance.

    if (bool(mVerticalScrollbar->getVisibility() & Visibility::FLAG_RENDER_NEEDED)) {
        auto prevScroll = mVerticalScrollbar->getCurrentScroll();
        mVerticalScrollbar->onScroll(event.delta.y);
        if (prevScroll != mVerticalScrollbar->getCurrentScroll()) {
            AWindow::current()->preventClickOnPointerRelease();
        }
    }
    if (bool(mHorizontalScrollbar->getVisibility() & Visibility::FLAG_RENDER_NEEDED)) {
        auto prevScroll = mHorizontalScrollbar->getCurrentScroll();
        mHorizontalScrollbar->onScroll(event.delta.x);
        if (prevScroll != mHorizontalScrollbar->getCurrentScroll()) {
            AWindow::current()->preventClickOnPointerRelease();
        }
    }
}

bool AScrollArea::onGesture(const glm::ivec2& origin, const AGestureEvent& event) {
    return AViewContainerBase::onGesture(origin, event);
}

void AScrollArea::onPointerPressed(const APointerPressedEvent& event) { AViewContainerBase::onPointerPressed(event); }

void AScrollArea::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainerBase::onPointerReleased(event);
}

void AScrollArea::scrollTo(ARect<int> target, bool nearestBorder) {
    const auto targetBegin = target.leftTop();
    const auto targetSize = target.size();
    const auto myBegin = getPositionInWindow();

    const auto toBeginPoint = targetBegin - myBegin;
    if (!nearestBorder) {
        scroll(toBeginPoint);
        return;
    }

    const auto targetEnd = targetBegin + targetSize * 2;
    const auto myEnd = myBegin + getSize();

    const auto toEndPoint = targetEnd - myEnd;

    auto delta = (targetBegin + targetSize / 2) - (myBegin + getSize() / 2);
    auto direction = glm::greaterThan(delta, glm::ivec2(0));
    const auto toBeginPointConditional = toBeginPoint * glm::ivec2(glm::greaterThan(myBegin, targetBegin));
    const auto toEndPointConditional = toEndPoint * glm::ivec2(glm::greaterThan(targetEnd, myEnd));
    scroll(glm::mix(toBeginPointConditional, toEndPointConditional, direction));
}
