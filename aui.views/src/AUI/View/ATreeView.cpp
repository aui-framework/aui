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

class ATreeView::ContainerView final: public AViewContainer {
private:
    int mScrollY = 0;
    size_t mIndex = -1;

public:
    void setScrollY(int scrollY) {
        mScrollY = scrollY;
        updateLayout();
    }

    void updateLayout() override {
        if (getLayout())
            getLayout()->onResize(mPadding.left, mPadding.top - mScrollY,
                                  getSize().x - mPadding.horizontal(), getSize().y - mPadding.vertical());
    }

    size_t getIndex() const {
        return mIndex;
    }

    int getContentMinimumWidth(ALayoutDirection layout) override {
        return 40;
    }

    int getContentMinimumHeight(ALayoutDirection layout) override {
        return 40;
    }
};

class ATreeView::ItemView: public AViewContainer, public ass::ISelectable
{
public:
    ItemView(ATreeView* treeView, const _<AView>& display, bool hasChildren, const ATreeIndex& index)
            : mDisplay(display),
              mIndex(index),
              mTreeView(treeView)
    {
        addAssName(".list-item");
        setLayout(_new<AHorizontalLayout>());

        if (hasChildren) {
            addView(mCollapseDisplay = _new<ADrawableView>(IDrawable::fromUrl(":uni/svg/tree-collapsed.svg")) let {
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

        connect(clicked, me::select);
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
            mCollapseDisplay->setDrawable(IDrawable::fromUrl(
                    mExpanded ? ":uni/svg/tree-expanded.svg" : ":uni/svg/tree-collapsed.svg"));
        }
        emit expandStateChanged(mExpanded);
    }

    void expand() {
        setExpanded(true);
    }

    void collapse() {
        setExpanded(true);
    }

    virtual ~ItemView() = default;

    const ATreeIndex& getIndex() const {
        return mIndex;
    }

    void setSelected(const bool selected)
    {
        if (selected) {
            AUI_NULLSAFE(mTreeView->mPrevSelection.lock())->setSelected(false);
            mTreeView->mPrevSelection = _cast<ItemView>(AView::sharedPtr());
            mTreeView->handleSelected(this);
        } else {
            mTreeView->mPrevSelection.reset();
        }
        mSelected = selected;
        emit customCssPropertyChanged;
    }

    void select() {
        setSelected(true);
    }

    void onPointerPressed(const APointerPressedEvent& event) override {
        AViewContainer::onPointerPressed(event);

        mTreeView->handleMousePressed(this);
    }

    void onPointerDoubleClicked(const APointerPressedEvent& event) override {
        AViewContainer::onPointerDoubleClicked(event);

        mTreeView->handleMouseDoubleClicked(this);
    }

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override {
        AViewContainer::onPointerMove(pos, event);
        mTreeView->handleMouseMove(this);
    }

    void setChildrenContainer(_<AViewContainer> childrenContainer) {
        mChildrenContainer = childrenContainer;
    }

    [[nodiscard]]
    const _<AViewContainer>& childrenContainer() const {
        return mChildrenContainer;
    }

signals:
    emits<bool> expandStateChanged;

protected:
    bool selectableIsSelectedImpl() override {
        return mSelected;
    }

private:
    _<AViewContainer> mChildrenContainer;

    bool mSelected = false;
    bool mExpanded = false;
    _<AView> mDisplay;
    _<ADrawableView> mCollapseDisplay;
    ATreeIndex mIndex;
    ATreeView* mTreeView;
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
    mContent->setExpanding();

    connect(mScrollbar->scrolled, mContent, &ContainerView::setScrollY);

    if (mModel) {
        fillViewsRecursively(mContent, model->root());
    }
    updateLayout();
    updateScrollbarDimensions();
    AWindow::current()->flagRedraw();
}

void ATreeView::makeElement(const _<AViewContainer>& container, const ATreeIndex& childIndex, bool isGroup, const _<ATreeView::ItemView>& itemView) {
    container->addView(itemView);

    // always add wrapper (even if isGroup = false) to simplify view walkthrough
    auto wrapper = _container<AVerticalLayout>({});
    wrapper->setVisibility(Visibility::GONE);
    wrapper << ".list-item-group";
    container->addView(wrapper);

    if (isGroup) {
        fillViewsRecursively(wrapper, childIndex);
        itemView->setChildrenContainer(wrapper);

        connect(itemView->expandStateChanged, wrapper, [this, wrapper](bool expanded) {
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


void ATreeView::setSize(glm::ivec2 size) {
    AViewContainer::setSize(size);

    updateScrollbarDimensions();
}

void ATreeView::updateScrollbarDimensions() {
    if (mContent) {
        mScrollbar->setScrollDimensions(getHeight(), mContent->AViewContainer::getContentMinimumHeight(
                ALayoutDirection::NONE));
    }
}

void ATreeView::onMouseScroll(const AScrollEvent& event) {
    //AViewContainer::onMouseScroll(pos, delta);
    mScrollbar->onMouseScroll(event);
    onPointerMove(event.origin, {event.pointerIndex}); // update hover on scroll
}


void ATreeView::handleMousePressed(ATreeView::ItemView* v) {
    emit itemMouseClicked(v->getIndex());
}

void ATreeView::handleMouseDoubleClicked(ATreeView::ItemView* v) {
    emit itemMouseDoubleClicked(v->getIndex());
}

void ATreeView::handleSelected(ATreeView::ItemView* v) {
    emit itemSelected(v->getIndex());
}

void ATreeView::fillViewsRecursively(const _<AViewContainer>& content, const ATreeIndex& index) {
    for (size_t i = 0; i < mModel->childrenCount(index); ++i) {
        auto childIndex = mModel->indexOfChild(i, 0, index);
        bool group = mModel->childrenCount(childIndex) != 0;
        auto item = _new<ItemView>(this, mViewFactory(mModel, childIndex), group, childIndex);
        makeElement(content, childIndex, group,  item);
    }

}

int ATreeView::getContentMinimumHeight(ALayoutDirection layout) {
    return 40;
}

void ATreeView::handleMouseMove(ATreeView::ItemView* pView) {
    emit itemMouseHover(pView->getIndex());
}

template<aui::invocable<std::size_t /* row */> Callable>
static void findRoot(const Callable& callable, const _<ITreeModel<AString>>& model, const ATreeIndex& indexToSelect) {
    if (!indexToSelect.hasValue()) {
        return;
    }
    findRoot(callable, model, model->parent(indexToSelect));
    callable(indexToSelect.row());
}

void ATreeView::select(const ATreeIndex& indexToSelect) {
    try {
        auto currentTarget = _cast<AViewContainer>(mContent);
        _<ATreeView::ItemView> itemView;
        bool ignore = true;
        findRoot([&](std::size_t row) {
            if (!currentTarget) {
                return;
            }
            if (ignore) {
                ignore = false;
                return;
            }
            auto c = _cast<ATreeView::ItemView>(currentTarget->getViews().at(row * 2));
            if (!c) {
                throw AException("bad item view");
            }
            itemView = c;
            c->expand();
            currentTarget = c->childrenContainer();
        }, mModel, indexToSelect);

        itemView->focus();
        itemView->setSelected(true);

        auto myPositionInWindow = getPositionInWindow();
        auto targetPositionInWindow = itemView->getPositionInWindow();

        mScrollbar->scroll(targetPositionInWindow.y - myPositionInWindow.y);

    } catch (const AException& e) {
        ALogger::warn("ATreeView") << "Failed to select view by index (unsynced model?): " << e;
    }
}

