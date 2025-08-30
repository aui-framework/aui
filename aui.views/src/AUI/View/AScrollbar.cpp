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

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/ASS/ASS.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AAbstractTextField.h>
#include "AScrollbar.h"
#include "ASpacerExpanding.h"


using namespace std::chrono_literals;

const _<ATimer>& AScrollbar::buttonTimer() {
    static _<ATimer> timer = _new<ATimer>(100ms);
    return timer;
}

AScrollbar::AScrollbar(ALayoutDirection direction) :
    mDirection(direction) {

    mForwardButton = _new<AScrollbarButton>();
    mBackwardButton = _new<AScrollbarButton>();

    connect(mForwardButton->pressed, me::scrollForward);
    connect(mBackwardButton->pressed, me::scrollBackward);

    switch (direction) {
        case ALayoutDirection::HORIZONTAL:
            setLayout(std::make_unique<AHorizontalLayout>());

            mForwardButton << ".scrollbar_right";
            mBackwardButton << ".scrollbar_left";
            break;
        case ALayoutDirection::VERTICAL:
            setLayout(std::make_unique<AVerticalLayout>());
            mForwardButton << ".scrollbar_down";
            mBackwardButton << ".scrollbar_up";
            break;
        case ALayoutDirection::NONE:
            break;
    }
    mHandle = aui::ptr::manage_shared(new AScrollbarHandle(*this));

    addView(mBackwardButton);
    addView(mOffsetSpacer = _new<AScrollbarOffsetSpacer>());
    addView(mHandle);
    addView(_new<ASpacerExpanding>());
    addView(mForwardButton);

    setScroll(0);
}

void AScrollbar::setOffset(size_t o) {
    switch (mDirection) {
        case ALayoutDirection::HORIZONTAL:
            mOffsetSpacer->setSize({o, 0});
            break;
        case ALayoutDirection::VERTICAL:
            mOffsetSpacer->setSize({0, o});
            break;
        case ALayoutDirection::NONE:
            break;
    }
}

void AScrollbar::setScrollDimensions(size_t viewportSize, size_t fullSize) {
    if (std::tie(mViewportSize, mFullSize) == std::tie(viewportSize, fullSize)) {
        return;
    }
    mViewportSize = viewportSize;
    mFullSize = fullSize;

    bool isOverflowing = mViewportSize < mFullSize;
    if (isOverflowing) {
        enable();
    }

    updateScrollHandleSize();
    bool shouldScrollToEnd = mStickToEnd && mStickToEnd->locked;
    setScroll(mCurrentScroll); // validates mCurrentScroll since mViewportSize and mFullSize are updated

    if (shouldScrollToEnd) {
        scrollToEnd();
    }
    emit updatedMaxScroll(getMaxScroll());

    if (!isOverflowing) {
        disable();
    }
}

void AScrollbar::updateScrollHandleSize() {
    using namespace ass;

    if (mAppearance == ScrollbarAppearance::NEVER) {
        setVisibility(Visibility::GONE);
        return;
    }

    if (mViewportSize >= mFullSize) {
        switch (mAppearance) {
            case ScrollbarAppearance::ALWAYS:
                setVisibility(Visibility::VISIBLE);
                mHandle->setVisibility(Visibility::GONE);
                break;
            case ScrollbarAppearance::ON_DEMAND:
                setVisibility(Visibility::GONE);
                break;
            case ScrollbarAppearance::NEVER:
                break;
        }
        return;
    }

    int scrollbarSpace = 0;

    switch (mDirection) {
        case ALayoutDirection::HORIZONTAL:
            scrollbarSpace = getWidth() - (mBackwardButton->getTotalOccupiedWidth() + mForwardButton->getTotalOccupiedWidth());
            break;
        case ALayoutDirection::VERTICAL:
            scrollbarSpace = getHeight() - (mBackwardButton->getTotalOccupiedHeight() + mForwardButton->getTotalOccupiedHeight());
            break;
        case ALayoutDirection::NONE:
            break;
    }
    scrollbarSpace = glm::max(scrollbarSpace, 0);

    auto handleSize = mFullSize > 0 ? int(scrollbarSpace * mViewportSize / mFullSize) : 0;

    setVisibility(Visibility::VISIBLE);
    mHandle->setVisibility(Visibility::VISIBLE);
    mHandle->setOverridenSize(handleSize);
    setEnabled();
    updateScrollHandleOffset(getMaxScroll());
}

void AScrollbar::setScroll(int scroll) {
    int max = getMaxScroll();
    auto newScroll = glm::clamp(scroll, 0, max);
    if (mCurrentScroll != newScroll) {
        mCurrentScroll = newScroll;

        updateScrollHandleOffset(max);

        emit scrolled(mCurrentScroll);
    }
    const bool scrolledToEnd = newScroll == max;
    if (mStickToEnd) {
        mStickToEnd->locked = scrolledToEnd;
    }
}

void AScrollbar::updateScrollHandleOffset(int max) {
    float availableSpace = getAvailableSpaceForSpacer();


    int handlePos = max > 0 ? float(mCurrentScroll) / max * availableSpace
                            : 0;

    handlePos = glm::max(handlePos, 0);

    switch (mDirection) {
        case ALayoutDirection::HORIZONTAL:
            mOffsetSpacer->setFixedSize(glm::ivec2{handlePos, 0});
            break;
        case ALayoutDirection::VERTICAL:
            mOffsetSpacer->setFixedSize(glm::ivec2{0, handlePos});
            break;
        case ALayoutDirection::NONE:
            break;
    }
    applyGeometryToChildren();
    redraw();
}

void AScrollbar::onScroll(float& delta) {
    // scroll 3 lines of text
    emit this->triggeredManually;
    auto prevScroll = this->getCurrentScroll();
    this->setScroll(mCurrentScroll + delta);
    auto newDelta = getCurrentScroll() - prevScroll;
    delta -= float(newDelta);
}

static int getButtonScrollSpeed() noexcept {
    if (AInput::isKeyDown(AInput::LSHIFT)) return 100;

    if (AInput::isKeyDown(AInput::LCONTROL)) return 1;

    return 10;
}

void AScrollbar::scrollForward() {
    setScroll(mCurrentScroll + getButtonScrollSpeed());
    buttonTimer()->start();
    connect(buttonTimer()->fired, this, [&] {
        if (AInput::isKeyDown(AInput::LBUTTON)) {
            setScroll(mCurrentScroll + getButtonScrollSpeed());
        } else {
            buttonTimer()->stop();
            AObject::disconnect();
        }
    });

    emit triggeredManually;
}

void AScrollbar::scrollBackward() {
    setScroll(mCurrentScroll - getButtonScrollSpeed());
    buttonTimer()->start();
    connect(buttonTimer()->fired, this, [&] {
        if (AInput::isKeyDown(AInput::LBUTTON)) {
            setScroll(mCurrentScroll - getButtonScrollSpeed());
        } else {
            buttonTimer()->stop();
            AObject::disconnect();
        }
    });

    emit triggeredManually;
}

void AScrollbar::onPointerPressed(const APointerPressedEvent& event) {
    AViewContainerBase::onPointerPressed(event);
}

void AScrollbar::handleScrollbar(int s) {
    setScroll(mCurrentScroll + s * int(getMaxScroll()) / getAvailableSpaceForSpacer());
}

float AScrollbar::getAvailableSpaceForSpacer() {

    switch (mDirection) {
        case ALayoutDirection::HORIZONTAL:
            return glm::max(0, getWidth() - (mBackwardButton->getTotalOccupiedWidth() + mForwardButton->getTotalOccupiedWidth() + mHandle->getTotalOccupiedWidth()));

        case ALayoutDirection::VERTICAL:
            return glm::max(0, getHeight() - (mBackwardButton->getTotalOccupiedHeight() + mForwardButton->getTotalOccupiedHeight() + mHandle->getTotalOccupiedHeight()));

        case ALayoutDirection::NONE:
            break;
    }
    return 0;
}

void AScrollbarHandle::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AView::onPointerMove(pos, event);
    if (mDragging) {
        switch (mScrollbar.mDirection) {
            case ALayoutDirection::HORIZONTAL:
                mScrollbar.handleScrollbar(pos.x - mScrollOffset);
                break;
            case ALayoutDirection::VERTICAL:
                mScrollbar.handleScrollbar(pos.y - mScrollOffset);
                break;
            case ALayoutDirection::NONE:
                break;
        }
    }
}

void AScrollbarHandle::onPointerPressed(const APointerPressedEvent& event) {
    AView::onPointerPressed(event);
    switch (mScrollbar.mDirection) {
        case ALayoutDirection::HORIZONTAL:
            mScrollOffset = event.position.x;
            break;
        case ALayoutDirection::VERTICAL:
            mScrollOffset = event.position.y;
            break;
        case ALayoutDirection::NONE:
            break;
    }

    mDragging = true;
    emit mScrollbar.triggeredManually;
}

void AScrollbarHandle::onPointerReleased(const APointerReleasedEvent& event) {
    AView::onPointerReleased(event);
    mDragging = false;
}

void AScrollbarHandle::setSize(glm::ivec2 size) {
    switch (mScrollbar.mDirection) {
        case ALayoutDirection::VERTICAL:
            size = {15_dp, mOverridenSize};
            break;

        case ALayoutDirection::HORIZONTAL:
            size = {mOverridenSize, 15_dp};
            break;
        case ALayoutDirection::NONE:
            break;
    }

    AView::setSize(size);
}

void AScrollbar::setSize(glm::ivec2 size) {
    AViewContainerBase::setSize(size);
    updateScrollHandleSize();
}

void AScrollbar::scrollToEnd() {
    setScroll(getMaxScroll());
    AUI_ASSERT(mCurrentScroll == getMaxScroll());
}

void AScrollbar::onScroll(const AScrollEvent& event) {
    AViewContainerBase::onScroll(event);
}
