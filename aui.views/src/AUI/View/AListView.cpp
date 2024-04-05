// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "AListView.h"

#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Util/UIBuildingHelpers.h>

#include "ALabel.h"
#include "AUI/ASS/Property/ScrollbarAppearance.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Enum/Visibility.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/Platform/AWindow.h"

class AListViewContainer : public AViewContainer {
   private:
    int mScrollY = 0;
    mutable std::size_t mIndex = -1;

   public:
    void updateLayout() override {
        if (getLayout())
            getLayout()->onResize(mPadding.left, mPadding.top - mScrollY, getSize().x - mPadding.horizontal(),
                                  getSize().y - mPadding.vertical());
        updateParentsLayoutIfNecessary();
    }

    _<AView> getViewAt(glm::ivec2 pos, ABitField<AViewLookupFlags> flags) const noexcept override {
        switch (mViews.size()) {
            case 0:
                return nullptr;
            case 1: {
                auto v = AViewContainer::getViewAt(pos, flags);
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

    size_t getIndex() const { return mIndex; }
};

class AListItem : public ALabel, public ass::ISelectable {
   private:
    bool mSelected = false;

   public:
    AListItem() { addAssName(".list-item"); }

    AListItem(const AString& text) : ALabel(text) { addAssName(".list-item"); }

    virtual ~AListItem() = default;

    void setSelected(const bool selected) {
        mSelected = selected;
        emit customCssPropertyChanged;
    }

   protected:
    bool selectableIsSelectedImpl() override { return mSelected; }

   public:
    void onPointerPressed(const APointerPressedEvent& event) override {
        AView::onPointerPressed(event);

        dynamic_cast<AListView*>(getParent()->getParent()->getParent())->handleMousePressed(this);
    }

    void onPointerDoubleClicked(const APointerPressedEvent& event) override {
        AView::onPointerDoubleClicked(event);

        dynamic_cast<AListView*>(getParent()->getParent()->getParent())->handleMouseDoubleClicked(this);
    }
};

AListView::~AListView() {}

AListView::AListView(const _<IListModel<AString>>& model) {
    mObserver = _new<AListModelObserver<AString>>(this);
    setModel(model);
}

void AListView::setModel(const _<IListModel<AString>>& model) {
    horizontalScrollbar()->setAppearance(ScrollbarAppearance::GONE);
    setContents(mContent = _new<AListViewContainer>());

    mContent->setLayout(_new<AVerticalLayout>());
    mContent->setExpanding();

    mObserver->setModel(model);
    if (model) {
        connect(model->dataRemoved, [&] { mSelectionModel.clear(); });
    }
}

void AListView::handleMousePressed(AListItem* item) {
    if (!(AInput::isKeyDown(AInput::LCONTROL) || AInput::isKeyDown(AInput::RCONTROL)) || !mAllowMultipleSelection) {
        clearSelectionInternal();
    }

    auto index = AListModelIndex(mContent->getIndex());
    if (mSelectionModel.contains(index))
    {
        mSelectionModel.erase(index);
        item->setSelected(false);
    }
    else
    {
        mSelectionModel << index;
        item->setSelected(true);
    }

    emit selectionChanged(getSelectionModel());
}

void AListView::clearSelectionInternal() {
    for (auto& s : mSelectionModel) {
        _cast<AListItem>(mContent->getViews()[s.getRow()])->setSelected(false);
    }

    mSelectionModel.clear();
}

void AListView::handleMouseDoubleClicked(AListItem* item) { emit itemDoubleClicked(mContent->getIndex()); }

void AListView::insertItem(size_t at, const AString& value) { mContent->addView(at, _new<AListItem>(value)); }

void AListView::updateItem(size_t at, const AString& value) {
    _cast<AListItem>(mContent->getViews()[at])->setText(value);
}

void AListView::removeItem(size_t at) { mContent->removeView(at); }

void AListView::onDataCountChanged() { AUI_NULLSAFE(AWindow::current())->flagUpdateLayout(); }

void AListView::onDataChanged() { redraw(); }

void AListView::selectItem(size_t i) {
    if (mAllowMultipleSelection) {
        mSelectionModel << AListModelIndex(i);
    } else {
        clearSelectionInternal();
        mSelectionModel = {AListModelIndex(i)};
    }

    _cast<AListItem>(mContent->getViews()[i])->setSelected(true);
    emit selectionChanged(getSelectionModel());
}

bool AListView::onGesture(const glm::ivec2& origin, const AGestureEvent& event) {
    return AViewContainer::onGesture(origin, event);
}

void AListView::setAllowMultipleSelection(bool allowMultipleSelection) {
    mAllowMultipleSelection = allowMultipleSelection;

    if (!allowMultipleSelection)
        clearSelection();
}

void AListView::clearSelection() {
    clearSelectionInternal();
    emit selectionChanged(getSelectionModel());
}