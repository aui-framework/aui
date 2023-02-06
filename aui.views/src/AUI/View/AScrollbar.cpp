// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
_<ATimer> AScrollbar::ourScrollButtonTimer = _new<ATimer>(100ms);

AScrollbar::AScrollbar(ALayoutDirection direction) :
    mDirection(direction) {

    mForwardButton = _new<AScrollbarButton>();
    mBackwardButton = _new<AScrollbarButton>();

    connect(mForwardButton->mousePressed, me::scrollForward);
    connect(mBackwardButton->mousePressed, me::scrollBackward);

    switch (direction) {
        case ALayoutDirection::HORIZONTAL:
            setLayout(_new<AHorizontalLayout>());

            mForwardButton << ".scrollbar_right";
            mBackwardButton << ".scrollbar_left";
            break;
        case ALayoutDirection::VERTICAL:
            setLayout(_new<AVerticalLayout>());
            mForwardButton << ".scrollbar_down";
            mBackwardButton << ".scrollbar_up";
            break;
    }
    mHandle = aui::ptr::manage(new AScrollbarHandle(*this));

    addView(mBackwardButton);
    addView(mOffsetSpacer = _new<AScrollbarOffsetSpacer>() let { it->setMinimumSize({0, 0}); });
    addView(mHandle);
    addView(_new<ASpacerExpanding>() let { it->setMinimumSize({0, 0}); });
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
    }
}

void AScrollbar::setScrollDimensions(size_t viewportSize, size_t fullSize) {
    bool shouldScrollToEnd = mStickToEnd && mFullSize < fullSize && mCurrentScroll == getMaxScroll();

    mViewportSize = viewportSize;
    mFullSize = fullSize;

    updateScrollHandleSize();

    if (shouldScrollToEnd) {
        scrollToEnd();
    }
}

void AScrollbar::updateScrollHandleSize() {
    float scrollbarSpace = 0;

    switch (mDirection) {
        case ALayoutDirection::HORIZONTAL:
            scrollbarSpace = getWidth() - (mBackwardButton->getTotalOccupiedWidth() + mForwardButton->getTotalOccupiedWidth());
            break;
        case ALayoutDirection::VERTICAL:
            scrollbarSpace = getHeight() - (mBackwardButton->getTotalOccupiedHeight() + mForwardButton->getTotalOccupiedHeight());
            break;
    }
    scrollbarSpace = glm::max(scrollbarSpace, 0.f);

    int o = mFullSize > 0 ? scrollbarSpace * mViewportSize / mFullSize
                          : 0;

    if (mAppearance == ScrollbarAppearance::NO_SCROLL_HIDE_CONTENT || mAppearance == ScrollbarAppearance::NO_SCROLL_SHOW_CONTENT) {
        setVisibility(Visibility::GONE);
        setEnabled(false);
    } else {
        if (o < scrollbarSpace) {
            setVisibility(Visibility::VISIBLE);
            mHandle->setVisibility(Visibility::VISIBLE);
            mHandle->setOverridenSize(o);
            setEnabled();
        } else {
            switch (mAppearance) {
                case ScrollbarAppearance::VISIBLE:
                    mHandle->setVisibility(Visibility::VISIBLE);
                    mHandle->setOverridenSize(scrollbarSpace);
                    break;
                case ScrollbarAppearance::INVISIBLE:
                    mHandle->setVisibility(Visibility::GONE);
                    break;
                case ScrollbarAppearance::GONE:
                    setVisibility(Visibility::GONE);
                    mHandle->setVisibility(Visibility::GONE);
                    break;
            }

            emit scrolled(mCurrentScroll = 0);
        }
    }

    updateScrollHandleOffset(getMaxScroll());
}

void AScrollbar::setScroll(int scroll) {
    updateScrollHandleSize();
    int max = getMaxScroll();
    auto newScroll = glm::clamp(scroll, 0, max);
    if (mCurrentScroll != newScroll) {
        mCurrentScroll = newScroll;

        updateScrollHandleOffset(max);


        emit scrolled(mCurrentScroll);
    }
}

void AScrollbar::updateScrollHandleOffset(int max) {
    float availableSpace = getAvailableSpaceForSpacer();


    int handlePos = max > 0 ? float(mCurrentScroll) / max * availableSpace
                            : 0;

    switch (mDirection) {
        case ALayoutDirection::HORIZONTAL:
            mOffsetSpacer->setFixedSize(glm::ivec2{handlePos, 0});
            break;
        case ALayoutDirection::VERTICAL:
            mOffsetSpacer->setFixedSize(glm::ivec2{0, handlePos});
            break;
    }
    updateLayout();
    redraw();
}

void AScrollbar::onScroll(glm::ivec2 pos, glm::ivec2 delta) {
    AViewContainer::onScroll(pos, delta);
    // scroll 3 lines of text
    setScroll(mCurrentScroll + delta.y * 11_pt * 3 / 120);
}

static int getButtonScrollSpeed() noexcept {
    if (AInput::isKeyDown(AInput::LSHIFT)) return 100;

    if (AInput::isKeyDown(AInput::LCONTROL)) return 1;

    return 10;
}

void AScrollbar::scrollForward() {
    setScroll(mCurrentScroll + getButtonScrollSpeed());
    ourScrollButtonTimer->start();
    connect(ourScrollButtonTimer->fired, this, [&] {
        if (AInput::isKeyDown(AInput::LBUTTON)) {
            setScroll(mCurrentScroll + getButtonScrollSpeed());
        } else {
            ourScrollButtonTimer->stop();
            AObject::disconnect();
        }
    });
}

void AScrollbar::scrollBackward() {
    setScroll(mCurrentScroll - getButtonScrollSpeed());
    ourScrollButtonTimer->start();
    connect(ourScrollButtonTimer->fired, this, [&] {
        if (AInput::isKeyDown(AInput::LBUTTON)) {
            setScroll(mCurrentScroll - getButtonScrollSpeed());
        } else {
            ourScrollButtonTimer->stop();
            AObject::disconnect();
        }
    });
}

void AScrollbar::onMousePressed(const APointerPressedEvent& event) {
    AViewContainer::onMousePressed(event);
}

void AScrollbar::handleScrollbar(int s) {
    setScroll(mCurrentScroll + s * int(getMaxScroll()) / getAvailableSpaceForSpacer());
}

float AScrollbar::getAvailableSpaceForSpacer() {

    switch (mDirection) {
        case ALayoutDirection::HORIZONTAL:
            return glm::max(0.f, getWidth() - (mBackwardButton->getTotalOccupiedWidth() + mForwardButton->getTotalOccupiedWidth() + mHandle->getTotalOccupiedWidth()));

        case ALayoutDirection::VERTICAL:
            return glm::max(0.f, getHeight() - (mBackwardButton->getTotalOccupiedHeight() + mForwardButton->getTotalOccupiedHeight() + mHandle->getTotalOccupiedHeight()));

    }
    return 0;
}

void AScrollbarHandle::onMouseMove(glm::ivec2 pos) {
    AView::onMouseMove(pos);
    if (mDragging) {
        switch (mScrollbar.mDirection) {
            case ALayoutDirection::HORIZONTAL:
                mScrollbar.handleScrollbar(pos.x - mScrollOffset);
                break;
            case ALayoutDirection::VERTICAL:
                mScrollbar.handleScrollbar(pos.y - mScrollOffset);
                break;
        }
    }
}

void AScrollbarHandle::onMousePressed(const APointerPressedEvent& event) {
    AView::onMousePressed(event);
    switch (mScrollbar.mDirection) {
        case ALayoutDirection::HORIZONTAL:
            mScrollOffset = event.position.x;
            break;
        case ALayoutDirection::VERTICAL:
            mScrollOffset = event.position.y;
            break;
    }

    mDragging = true;
}

void AScrollbarHandle::onMouseReleased(const APointerReleasedEvent& event) {
    AView::onMouseReleased(event);
    mDragging = false;
}

void AScrollbarHandle::setSize(glm::ivec2 size) {
    switch (mScrollbar.mDirection) {
        case ALayoutDirection::VERTICAL:
            size.y = mOverridenSize;
            break;

        case ALayoutDirection::HORIZONTAL:
            size.x = mOverridenSize;
            break;
    }

    AView::setSize(size);
}

void AScrollbar::setSize(glm::ivec2 size) {
    AViewContainer::setSize(size);
    updateScrollHandleSize();
}
