/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 06.12.2020.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/ASS/ASS.h>
#include "AScrollbar.h"
#include "ASpacer.h"

class AScrollbarButton: public AView {
public:
    AScrollbarButton() {

    }
};
class AScrollbarHandle: public AView {
public:
    AScrollbarHandle() {

    }

    int getMinimumWidth() override {
        return 0;
    }

    int getMinimumHeight() override {
        return 0;
    }
};
class AScrollbarOffsetSpacer: public ASpacer {
public:
    AScrollbarOffsetSpacer(): ASpacer(0, 0) {

    }

    int getMinimumWidth() override {
        return 0;
    }

    int getMinimumHeight() override {
        return 0;
    }
};

AScrollbar::AScrollbar(LayoutDirection direction) : mDirection(direction) {

    mForwardButton = _new<AScrollbarButton>();
    mBackwardButton = _new<AScrollbarButton>();

    switch (direction) {
        case LayoutDirection::HORIZONTAL:
            setLayout(_new<AHorizontalLayout>());

            mForwardButton->setCustomAss({
                ass::BackgroundImage {
                    ":uni/svg/sb-right.svg"
                },
                ass::FixedSize { 17_dp, 15_dp }
            });
            mBackwardButton->setCustomAss({
                ass::BackgroundImage {
                    ":uni/svg/sb-left.svg"
                },
                ass::FixedSize { 17_dp, 15_dp }
            });
            break;
        case LayoutDirection::VERTICAL:
            setLayout(_new<AVerticalLayout>());
            mForwardButton->setCustomAss({
                 ass::BackgroundImage {
                         ":uni/svg/sb-down.svg"
                 },
                 ass::FixedSize { 15_dp, 17_dp }
            });
            mBackwardButton->setCustomAss({
                 ass::BackgroundImage {
                         ":uni/svg/sb-top.svg"
                 },
                 ass::FixedSize { 15_dp, 17_dp }
            });
            break;
    }
    mHandle = _new<AScrollbarHandle>();

    addView(mBackwardButton);
    addView(mOffsetSpacer = _new<AScrollbarOffsetSpacer>() let (ASpacer, {setMinimumSize({0, 0});}));
    addView(mHandle);
    addView(_new<ASpacer>() let (ASpacer, {setMinimumSize({0, 0});}));
    addView(mForwardButton);

    setScroll(0);
}

void AScrollbar::setOffset(size_t o) {
    switch (mDirection) {
        case LayoutDirection::HORIZONTAL:
            mOffsetSpacer->setSize(o, 0);
            break;
        case LayoutDirection::VERTICAL:
            mOffsetSpacer->setSize(0, o);
            break;
    }
}

void AScrollbar::setScrollDimensions(size_t viewportSize, size_t fullSize) {
    mViewportSize = viewportSize;
    mFullSize = fullSize;

    updateScrollHandleSize();
}

void AScrollbar::updateScrollHandleSize() {
    float scrollbarSpace = 0;

    switch (mDirection) {
        case LayoutDirection::HORIZONTAL:
            scrollbarSpace = getWidth() + mBackwardButton->getTotalOccupiedWidth() + mForwardButton->getTotalOccupiedWidth() + mHandle->getMargin().horizontal();
            break;
        case LayoutDirection::VERTICAL:
            scrollbarSpace = getHeight() + mBackwardButton->getTotalOccupiedHeight() + mForwardButton->getTotalOccupiedHeight() + mHandle->getMargin().vertical();
            break;
    }

    size_t o = glm::max(float(10_dp), scrollbarSpace * mViewportSize / mFullSize);

    if (o < scrollbarSpace) {
        setEnabled();
        mHandle->setVisibility(Visibility::VISIBLE);
        switch (mDirection) {
            case LayoutDirection::HORIZONTAL:
                mHandle->setFixedSize({o, mHandle->getHeight()});
                break;
            case LayoutDirection::VERTICAL:
                mHandle->setFixedSize({mHandle->getWidth(), o});
                break;
        }
    } else {
        mHandle->setVisibility(Visibility::GONE);
        setDisabled();
    }
}

void AScrollbar::setScroll(int scroll) {
    auto newScroll = glm::clamp(scroll, 0, int(mFullSize - mViewportSize));
    if (mCurrentScroll != newScroll) {
        mCurrentScroll = newScroll;

        int handlePos = float(mCurrentScroll) * float(mViewportSize) / mFullSize;

        switch (mDirection) {
            case LayoutDirection::HORIZONTAL:
                mOffsetSpacer->setFixedSize(glm::ivec2{handlePos, 0});
                break;
            case LayoutDirection::VERTICAL:
                mOffsetSpacer->setFixedSize(glm::ivec2{0, handlePos});
                break;
        }
        updateLayout();
        redraw();
        emit scrolled(mCurrentScroll);
    }
}

void AScrollbar::onMouseWheel(glm::ivec2 pos, int delta) {
    AViewContainer::onMouseWheel(pos, delta);
    setScroll(mCurrentScroll + delta);
}
