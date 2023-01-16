// AUI Framework - Declarative UI toolkit for modern C++17
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

void AScrollArea::setSize(glm::ivec2 size) {
    AViewContainer::setSize(size);
    if (mContentContainer->hasChild()) {
        mVerticalScrollbar->setScrollDimensions(
                mContentContainer->getContentHeight() + mContentContainer->getTotalFieldVertical(),
                mContentContainer->child()->getContentMinimumHeight(ALayoutDirection::NONE));
    }
}

void AScrollArea::onMouseWheel(glm::ivec2 pos, glm::ivec2 delta) {
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

