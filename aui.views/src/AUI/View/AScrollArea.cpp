/*
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2772 on 12/6/20.
//

#include "AScrollArea.h"
#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/UIBuildingHelpers.h>

class AScrollAreaContainer: public AViewContainer {
private:
    glm::uvec2 mScroll = {0, 0}; // TODO horizontal

public:
    AScrollAreaContainer() {
        addAssName(".scrollarea_inner");
    }

    int getContentMinimumWidth(ALayoutDirection layout) override {
        return 30_dp;
    }

    void updateLayout() override {
        if (hasChild()) child()->setGeometry(0, -mScroll.y, getContentWidth(), getContentHeight() + mScroll.y);
    }

    int getContentMinimumHeight(ALayoutDirection layout) override {
        return 30_dp;
    }

    void setScrollY(unsigned scroll) {
        mScroll.y = scroll;
        updateLayout();
    }

    void setContent(const _<AView>& view) {
        removeAllViews();
        addView(view);
    }

    bool hasChild() const noexcept {
        return !getViews().empty();
    }

    [[nodiscard]]
    const _<AView>& child() const noexcept {
        assert(("no scrollarea child specified", !getViews().empty()));
        assert(("scrollarea can have only one child", getViews().size() == 1));
        return getViews().first();
    }
};

AScrollArea::AScrollArea():
    AScrollArea(Builder{})
{
}

AScrollArea::AScrollArea(const AScrollArea::Builder& builder) {
    setLayout(_new<AAdvancedGridLayout>(2, 2));

    auto contentContainer = _new<AScrollAreaContainer>();
    mContentContainer = contentContainer;
    addView(contentContainer);
    if (!builder.mExternalVerticalScrollbar) {
        addView(mVerticalScrollbar = _new<AScrollbar>(ALayoutDirection::VERTICAL));
    } else {
        mVerticalScrollbar = builder.mExternalVerticalScrollbar;
    }
    addView(mHorizontalScrollbar = _new<AScrollbar>(ALayoutDirection::HORIZONTAL));

    mHorizontalScrollbar->setVisibility(Visibility::GONE);
    mContentContainer->setExpanding();

    if (builder.mContents) {
        setContents(builder.mContents);
    }

    setExpanding();

    connect(mVerticalScrollbar->scrolled, slot(contentContainer)::setScrollY);
}

AScrollArea::~AScrollArea() = default;

void AScrollArea::setSize(int width, int height) {
    AViewContainer::setSize(width, height);
    if (mContentContainer->hasChild()) {
        mVerticalScrollbar->setScrollDimensions(
                mContentContainer->getContentHeight() + mContentContainer->getTotalFieldVertical(),
                mContentContainer->child()->getContentMinimumHeight(ALayoutDirection::NONE));
    }
}

void AScrollArea::onMouseWheel(const glm::ivec2& pos, const glm::ivec2& delta) {
    AViewContainer::onMouseWheel(pos, delta);
    mVerticalScrollbar->onMouseWheel(pos, delta);
}

int AScrollArea::getContentMinimumHeight(ALayoutDirection layout) {
    return 30;
}

_<AViewContainer> AScrollArea::getContentContainer() const {
    return mContentContainer;
}

void AScrollArea::setContents(const _<AViewContainer>& container) {
    mContentContainer->setContent(container);
}

bool AScrollArea::onGesture(const glm::ivec2 &origin, const AGestureEvent &event) {
    if (std::holds_alternative<AFingerDragEvent>(event)) {
        if (transformGestureEventsToDesktop(origin, event)) {
            return true;
        }
    }
    return AViewContainer::onGesture(origin, event);
}
