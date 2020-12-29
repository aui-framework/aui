//
// Created by alex2772 on 12/6/20.
//

#include "AScrollArea.h"
#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Util/kAUI.h>

class AScrollAreaContainer: public AViewContainer {
private:
    int mScroll = 0;

public:

    int getContentMinimumWidth() override {
        return 30_dp;
    }

    void updateLayout() override {
        if (auto l = getLayout())
            l->onResize(mPadding.left, mPadding.top - mScroll,
                              getSize().x - mPadding.horizontal(), getSize().y - mPadding.vertical());
        updateParentsLayoutIfNecessary();
    }

    int getContentMinimumHeight() override {
        return 30_dp;
    }

    void setScrollY(int scroll) {
        mScroll = scroll;
        updateLayout();
    }

};

AScrollArea::AScrollArea() {
    setLayout(_new<AAdvancedGridLayout>(2, 2));

    auto contentContainer = _new<AScrollAreaContainer>();
    mContentContainer = contentContainer;
    addView(contentContainer);
    addView(mVerticalScrollbar = _new<AScrollbar>(LayoutDirection::VERTICAL));
    addView(mHorizontalScrollbar = _new<AScrollbar>(LayoutDirection::HORIZONTAL));

    mContentContainer->setCss("overflow: hidden");
    mContentContainer->setExpanding({2, 2});

    setExpanding({2, 2});


    connect(mVerticalScrollbar->scrolled, slot(contentContainer)::setScrollY);
}

AScrollArea::~AScrollArea() = default;

void AScrollArea::setSize(int width, int height) {
    AViewContainer::setSize(width, height);
    mVerticalScrollbar->setScrollDimensions(mContentContainer->getContentHeight() + mContentContainer->getTotalFieldVertical(), mContentContainer->AViewContainer::getContentMinimumHeight());
}

void AScrollArea::onMouseWheel(glm::ivec2 pos, int delta) {
    AViewContainer::onMouseWheel(pos, delta);
    mVerticalScrollbar->onMouseWheel(pos, delta);
}
