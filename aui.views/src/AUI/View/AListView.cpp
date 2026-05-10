/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "AListView.h"

#include <AUI/ASS/Property/ScrollbarAppearance.h>
#include <AUI/ASS/ASS.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Enum/Visibility.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Util/UIBuildingHelpers.h>

#include "ALabel.h"

class AListViewContainer : public AViewContainer {
   private:
    int mScrollY = 0;
    mutable std::size_t mIndex = -1;

   public:
    void onLayout(int w, int h) override {
        if (getLayout())
            getLayout()->layout(mPadding.left, mPadding.top - mScrollY, getSize().x - mPadding.horizontal(),
                                  getSize().y - mPadding.vertical());
    }

    _<AView> getViewAt(glm::ivec2 pos, ABitField<AViewLookupFlags> flags) const noexcept override {
        switch (getViews().size()) {
            case 0:
                return nullptr;
            case 1: {
                auto v = AViewContainerBase::getViewAt(pos, flags);
                mIndex = v ? 0 : -1;
                return v;
            }
            default: {
                mIndex = -1;
                pos.y += mScrollY;
                pos.y /= getViews()[1]->getPosition().y - getViews()[0]->getPosition().y;
                if (pos.y >= 0 && pos.y < getViews().size()) {
                    return getViews()[mIndex = pos.y];
                }
                return nullptr;
            }
        }
    }

    void setScrollY(int scrollY) {
        mScrollY = scrollY;
        onLayout(getWidth(), getHeight());
    }

    size_t getIndex() const { return mIndex; }
};

class AListItem : public ALabel, public ass::ISelectable {
   private:
    bool mSelected = false;

   public:
    AListItem() { addAssName(".list-item"); }

    AListItem(const AString& text) : ALabel(text) {
        addAssName(".list-item");
        setExpanding({1, 0});
    }

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

        dynamic_cast<AListView*>(getParent()->getParent()->getParent()->getParent())->handleMousePressed(this);
    }

    void onPointerDoubleClicked(const APointerPressedEvent& event) override {
        AView::onPointerDoubleClicked(event);

        dynamic_cast<AListView*>(getParent()->getParent()->getParent()->getParent())->handleMouseDoubleClicked(this);
    }
};

AListView::~AListView() {}

AListView::AListView(_<IListModel<AString>> model) {
    setCustomStyle({
        ass::MinSize { AMetric(0, AMetric::T_PX), AMetric(0, AMetric::T_PX) },
    });
    mObserver = _new<AListModelObserver<AString>>(this);
    setModel(std::move(model));
}

void AListView::setModel(_<IListModel<AString>> model) {
    horizontalScrollbar()->setAppearance(ass::ScrollbarAppearance::NEVER);
    setContents(mContent = _new<AListViewContainer>());

    mContent->setLayout(std::make_unique<AVerticalLayout>());
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
    if (mSelectionModel.contains(index)) {
        mSelectionModel.erase(index);
        item->setSelected(false);
    } else {
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
    _cast<AListItem>(mContent->getViews()[at])->text() = value;
}

void AListView::removeItem(size_t at) { mContent->removeView(at); }

void AListView::onDataCountChanged() { requestLayout(); }

void AListView::onDataChanged() { redraw(); }

glm::ivec2 AListView::onIntrinsicMeasure(AConstraints constraints) {
  if (!mContent) {
    return { 0, 0 };
  }
  const int width = constraints.isUnlimitedWidth()
      ? constraints.minWidth
      : constraints.maxWidth;
  const int contentWidth = glm::max(0, width);
  const auto contentMeasured = mContent->measure(AConstraints::fixedWidth(contentWidth));
  const int maxWidth = constraints.isUnlimitedWidth() ? std::numeric_limits<int>::max() : constraints.maxWidth;
  const int maxHeight = constraints.isUnlimitedHeight() ? std::numeric_limits<int>::max() : constraints.maxHeight;
  return {
    std::clamp(width, constraints.minWidth, maxWidth),
    std::clamp(contentMeasured.y, constraints.minHeight, maxHeight),
  };
}

AMinMaxAxis AListView::onComputeIntrinsicMinMaxAxis(int) {
    return {
        .min = 0,
        .max = 0,
    };
}

void AListView::updateSelectionOnItem(size_t i, AListView::SelectAction action) {
    switch (action) {
        case SelectAction::CLEAR_SELECTION_AND_SET:
            clearSelectionInternal();
            mSelectionModel = {AListModelIndex(i)};
            _cast<AListItem>(mContent->getViews()[i])->setSelected(true);
            break;
        case SelectAction::SET:
            if (mAllowMultipleSelection) {
                mSelectionModel << AListModelIndex(i);
            } else {
                clearSelectionInternal();
                mSelectionModel = {AListModelIndex(i)};
            }
            _cast<AListItem>(mContent->getViews()[i])->setSelected(true);
            break;
        case SelectAction::UNSET:
            mSelectionModel.erase(i);
            _cast<AListItem>(mContent->getViews()[i])->setSelected(false);
            break;
        case SelectAction::TOGGLE:
            if (mSelectionModel.contains(i)) {
                updateSelectionOnItem(i, SelectAction::UNSET);
            } else {
                updateSelectionOnItem(i, SelectAction::SET);
            }
            break;
    }

    emit selectionChanged(getSelectionModel());
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
