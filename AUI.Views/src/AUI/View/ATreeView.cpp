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
#include "ADrawableView.h"
#include <AUI/ASS/ASS.h>
#include <AUI/Util/UIBuildingHelpers.h>

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

    void setScrollY(int scrollY) {
        mScrollY = scrollY;
        updateLayout();
    }

    size_t getIndex() const {
        return mIndex;
    }

    int getContentMinimumWidth() override {
        return 40;
    }

    int getContentMinimumHeight() override {
        return 40;
    }
};
class ATreeView::ItemView: public AViewContainer
{
private:
    bool mSelected = false;
    bool mExpanded = false;
    _<AView> mDisplay;
    _<ADrawableView> mCollapseDisplay;
    ATreeIndex mIndex;
    ATreeView* mTreeView;

public:
    ItemView(ATreeView* treeView, const _<AView>& display, bool hasChildren, const ATreeIndex& index)
            : mDisplay(display),
              mIndex(index),
              mTreeView(treeView)
    {
        addAssName(".list-item");
        setLayout(_new<AHorizontalLayout>());

        if (hasChildren) {
            addView(mCollapseDisplay = _new<ADrawableView>(AImageLoaderRegistry::inst().loadDrawable(":uni/svg/tree-collapsed.svg")) let {
                it << ".list-item-icon";
                connect(it->clicked, me::toggleCollapse);
            });
        } else {
            addView(_new<AView>() << ".list-item-icon");
        }

        addView(mDisplay);
        if (hasChildren) {
            connect(mDisplay->doubleClicked, me::toggleCollapseRecursive);
        }
    }

    void toggleCollapse() {
        setExpanded(!mExpanded);
    }
    void toggleCollapseRecursive() {
        expandOrCollapseRecursively(!mExpanded);
    }

    void expandOrCollapseRecursively(bool expand) {
        setExpanded(expand);
        auto& parentViews = getParent()->getViews();
        for (auto it = parentViews.begin(); it != parentViews.end(); ++it) {
            if (it->get() == this) {
                ++it;
                if (it != parentViews.end()) {
                    if (auto container = _cast<AViewContainer>(*it)) {
                        for (auto& v : container->getViews()) {
                            if (auto treeItem = _cast<ItemView>(v)) {
                                treeItem->expandOrCollapseRecursively(expand);
                            }
                        }
                    }
                }
                break;
            }
        }
    }

    void setExpanded(bool expanded) {
        mExpanded = expanded;
        if (mCollapseDisplay) {
            mCollapseDisplay->setDrawable(AImageLoaderRegistry::inst().loadDrawable(
                    mExpanded ? ":uni/svg/tree-expanded.svg" : ":uni/svg/tree-collapsed.svg"));
        }
        emit expandStateChanged(mExpanded);
    }

    virtual ~ItemView() = default;

    const ATreeIndex& getIndex() const {
        return mIndex;
    }

    void getCustomCssAttributes(AMap<AString, AVariant>& map) override
    {
        AViewContainer::getCustomCssAttributes(map);
        if (mSelected)
            map["selected"] = true;
    }

    void setSelected(const bool selected)
    {
        mSelected = selected;
        emit customCssPropertyChanged;
    }

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override {
        AViewContainer::onMousePressed(pos, button);

        mTreeView->handleMousePressed(this);
    }

    void onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) override {
        AViewContainer::onMouseDoubleClicked(pos, button);

        mTreeView->handleMouseDoubleClicked(this);
    }

    void onMouseMove(glm::ivec2 pos) override {
        AViewContainer::onMouseMove(pos);
        mTreeView->handleMouseMove(this);
    }

signals:
    emits<bool> expandStateChanged;
};


ATreeView::ATreeView():
    mViewFactory([](const _<ITreeModel<AString>>& model, const ATreeIndex& index) {
        return _new<ALabel>(model->itemAt(index));
    })
{

}

ATreeView::ATreeView(const _<ITreeModel<AString>>& model):
    ATreeView()
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
        for (size_t i = 0; i < model->childrenCount(model->root()); ++i) {
            ATreeIndex index = {model->getUserDataForRoot(), i, 0};
            ATreeIndex childIndex = mModel->indexOfChild(i, 0, index);
            bool group = model->childrenCount(childIndex) != 0;

            auto item = _new<ItemView>(this, mViewFactory(mModel, index), group, index);
            makeElement(mContent, childIndex, group, item);
        }
    }
    updateLayout();
    updateScrollbarDimensions();
    AWindow::current()->flagRedraw();
}

void ATreeView::makeElement(const _<AViewContainer>& container, const ATreeIndex& childIndex, bool isGroup, const _<ATreeView::ItemView>& itemView) {
    container->addView(itemView);
    if (isGroup) {
        auto wrapper = _container<AVerticalLayout>({});
        wrapper->setVisibility(Visibility::GONE);
        wrapper << ".list-item-group";
        container->addView(wrapper);
        fillViewsRecursively(wrapper, childIndex);

        connect(itemView->expandStateChanged, wrapper, [&, wrapper](bool expanded) {
            if (expanded) {
                wrapper->setVisibility(Visibility::VISIBLE);
            } else {
                wrapper->setVisibility(Visibility::GONE);
            }

            updateLayout();
            updateScrollbarDimensions();
            redraw();
        });
    }
}


void ATreeView::setSize(int width, int height) {
    AViewContainer::setSize(width, height);

    updateScrollbarDimensions();
}

void ATreeView::updateScrollbarDimensions() {
    if (mContent) {
        mScrollbar->setScrollDimensions(getHeight(), mContent->AViewContainer::getContentMinimumHeight());
    }
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

void ATreeView::fillViewsRecursively(const _<AViewContainer>& content, const ATreeIndex& index) {
    for (size_t i = 0; i < mModel->childrenCount(index); ++i) {
        auto childIndex = mModel->indexOfChild(i, 0, index);
        bool group = mModel->childrenCount(childIndex) != 0;
        auto item = _new<ItemView>(this, mViewFactory(mModel, childIndex), group, childIndex);
        makeElement(content, childIndex, group,  item);
    }

}

int ATreeView::getContentMinimumHeight() {
    return 40;
}

void ATreeView::handleMouseMove(ATreeView::ItemView* pView) {
    emit itemMouseHover(pView->getIndex());
}
