//
// Created by alex2 on 06.12.2020.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include "AScrollbar.h"
#include "ASpacer.h"

class AScrollbarButton: public AView {
public:
    AScrollbarButton() {
        AVIEW_CSS;
    }
};
class AScrollbarHandle: public AView {
public:
    AScrollbarHandle() {
        AVIEW_CSS;
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
    AVIEW_CSS;
    mForwardButton = _new<AScrollbarButton>();
    mBackwardButton = _new<AScrollbarButton>();

    switch (direction) {
        case LayoutDirection::HORIZONTAL:
            setLayout(_new<AHorizontalLayout>());
            mForwardButton->setCss("background: url(':win/svg/sb-right.svg');"
                                  "width: 17em;"
                                  "height: 15em;");
            mBackwardButton->setCss("background: url(':win/svg/sb-left.svg');"
                                   "width: 17em;"
                                   "height: 15em;");
            break;
        case LayoutDirection::VERTICAL:
            setLayout(_new<AVerticalLayout>());
            mForwardButton->setCss("background: url(':win/svg/sb-down.svg');"
                                  "width: 15em;"
                                  "height: 17em;");
            mBackwardButton->setCss("background: url(':win/svg/sb-top.svg');"
                                   "width: 15em;"
                                   "height: 17em;");
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

    size_t o = glm::max(10_dp, scrollbarSpace * mViewportSize / mFullSize);

    if (o < scrollbarSpace) {
        setEnabled();
        mHandle->setVisibility(V_VISIBLE);
        switch (mDirection) {
            case LayoutDirection::HORIZONTAL:
                mHandle->setFixedSize({o, mHandle->getHeight()});
                break;
            case LayoutDirection::VERTICAL:
                mHandle->setFixedSize({mHandle->getWidth(), o});
                break;
        }
    } else {
        mHandle->setVisibility(V_GONE);
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
