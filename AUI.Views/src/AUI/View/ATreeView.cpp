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
// Created by alex2 on 7/1/2021.
//


#include "ATreeView.h"
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Platform/AWindow.h>
#include "ALabel.h"

class ATreeView::ContainerView: public AViewContainer {
private:
    int mScrollY = 0;
    size_t mIndex = -1;

public:
    void updateLayout() override {
        if (getLayout())
            getLayout()->onResize(mPadding.left, mPadding.top - mScrollY,
                                  getSize().x - mPadding.horizontal(), getSize().y - mPadding.vertical());
        updateParentsLayoutIfNecessary();
    }

    _<AView> getViewAt(glm::ivec2 pos, bool ignoreGone) override {
        switch (mViews.size()) {
            case 0:
                return nullptr;
            case 1: {
                auto v = AViewContainer::getViewAt(pos, ignoreGone);
                mIndex = v ? 0 : -1;
                return v;
            }
            default: {
                mIndex = -1;
                pos.y += mScrollY;
                pos.y /= mViews[1]->getPosition().y - mViews[0]->getPosition().y;
                if (pos.y >= 0 && pos.y < mViews.size()) {
                    return mViews[mIndex = pos.y];
                }
                return nullptr;
            }
        }
    }

    void setScrollY(int scrollY) {
        mScrollY = scrollY;
        updateLayout();
    }

    size_t getIndex() const {
        return mIndex;
    }
};
class ATreeView::ItemView: public ALabel
{
private:
    bool mSelected = false;

public:
    ItemView()
    {

    }

    ItemView(const AString& text)
            : ALabel(text)
    {
    }

    virtual ~ItemView() = default;

    void getCustomCssAttributes(AMap<AString, AVariant>& map) override
    {
        ALabel::getCustomCssAttributes(map);
        if (mSelected)
            map["selected"] = true;
    }

    void setSelected(const bool selected)
    {
        mSelected = selected;
        emit customCssPropertyChanged;
    }

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override {
        AView::onMousePressed(pos, button);

        dynamic_cast<ATreeView*>(getParent()->getParent())->handleMousePressed(this);
    }

    void onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) override {
        AView::onMouseDoubleClicked(pos, button);

        dynamic_cast<ATreeView*>(getParent()->getParent())->handleMouseDoubleClicked(this);
    }
};



ATreeView::ATreeView() {

}

ATreeView::ATreeView(const _<ITreeModel<AString>>& model)
{
    setModel(model);
}

void ATreeView::setModel(const _<ITreeModel<AString>>& model) {
    mModel = model;
    setLayout(_new<AHorizontalLayout>());

    addView(mContent = _new<ContainerView>());
    addView(mScrollbar = _new<AScrollbar>());

    mContent->setLayout(_new<AVerticalLayout>());
    mContent->setExpanding({2, 2});

    connect(mScrollbar->scrolled, mContent, &ContainerView::setScrollY);

    if (mModel) {
        for (size_t i = 0; i < model->rowCount({}); ++i) {
            mContent->addView(_new<ItemView>(model->itemAt(i)));
        }
    }
    updateLayout();
    updateScrollbarDimensions();
    AWindow::current()->flagRedraw();
}


void ATreeView::setSize(int width, int height) {
    AViewContainer::setSize(width, height);

    updateScrollbarDimensions();
}

void ATreeView::updateScrollbarDimensions() {
    mScrollbar->setScrollDimensions(getHeight(), mContent->getContentMinimumHeight());
}

void ATreeView::onMouseWheel(glm::ivec2 pos, int delta) {
    //AViewContainer::onMouseWheel(pos, delta);
    mScrollbar->onMouseWheel(pos, delta);
    onMouseMove(pos); // update hover on scroll
}

void ATreeView::handleMousePressed(ATreeView::ItemView* v) {

}

void ATreeView::handleMouseDoubleClicked(ATreeView::ItemView* v) {

}
