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
#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/UIBuildingHelpers.h>


AScrollArea::AScrollArea():
    AScrollArea(Builder{})
{
    addAssName("AScrollArea");
}

AScrollArea::AScrollArea(const AScrollArea::Builder& builder) {
    addAssName("AScrollArea");
    setLayout(std::make_unique<AAdvancedGridLayout>(2, 2));

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

    connect(mVerticalScrollbar->scrolled, slot(mInner)::setScrollY);
    connect(mHorizontalScrollbar->scrolled, slot(mInner)::setScrollX);
}

int AScrollArea::getContentMinimumWidth() {
    if (getExpandingHorizontal() != 0) return 0;
    return AViewContainerBase::getContentMinimumWidth() + (contents() ? contents()->getMinimumSizePlusMargin().x : 0);
}
int AScrollArea::getContentMinimumHeight() {
    if (getExpandingVertical() != 0) return 0;
    return AViewContainerBase::getContentMinimumHeight() + (contents() ? contents()->getMinimumSizePlusMargin().y : 0);
}
void AScrollArea::setSize(glm::ivec2 size) {
    AViewContainerBase::setSize(size);
    mInner->applyGeometryToChildrenIfNecessary();
    if (contents()) {
        mVerticalScrollbar->setScrollDimensions(
                mInner->getHeight(),
                contents()->getMinimumSizePlusMargin().y);

        mHorizontalScrollbar->setScrollDimensions(
                mInner->getWidth(),
                contents()->getMinimumSizePlusMargin().x);
    }
}

void AScrollArea::onScroll(const AScrollEvent& event) {
    AViewContainerBase::onScroll(event);
    if (!mIsWheelScrollable) {
        return;
    }

    {
        auto prevScroll = mVerticalScrollbar->getCurrentScroll();
        mVerticalScrollbar->onScroll(event.delta.y);
        if (prevScroll != mVerticalScrollbar->getCurrentScroll()) {
            AWindow::current()->preventClickOnPointerRelease();
        }
    }
    {
        auto prevScroll = mHorizontalScrollbar->getCurrentScroll();
        mHorizontalScrollbar->onScroll(event.delta.x);
        if (prevScroll != mHorizontalScrollbar->getCurrentScroll()) {
            AWindow::current()->preventClickOnPointerRelease();
        }
    }
}

bool AScrollArea::onGesture(const glm::ivec2 &origin, const AGestureEvent &event) {
    return AViewContainerBase::onGesture(origin, event);
}

void AScrollArea::onPointerPressed(const APointerPressedEvent& event) {
    AViewContainerBase::onPointerPressed(event);
}

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
