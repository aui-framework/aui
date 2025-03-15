/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "AListView.h"

#include <AUI/ASS/Property/ScrollbarAppearance.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Enum/Visibility.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>

#include "ALabel.h"

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

        dynamic_cast<AListView*>(getParent()->getParent()->getParent()->getParent())->handleMousePressed(this);
    }

    void onPointerDoubleClicked(const APointerPressedEvent& event) override {
        AView::onPointerDoubleClicked(event);

        dynamic_cast<AListView*>(getParent()->getParent()->getParent()->getParent())->handleMouseDoubleClicked(this);
    }
};

AListView::~AListView() {}

AListView::AListView(_<IListModel<AString>> model) {
    horizontalScrollbar()->setAppearance(ass::ScrollbarAppearance::NEVER);
    AScrollArea::setContents(mForEachUI = AUI_DECLARATIVE_FOR(i, std::move(model), AVerticalLayout) {
        return _new<AListItem>(i);
    });
}

void AListView::setModel(_<IListModel<AString>> model) {
    if (auto prevModel = mForEachUI->model()) {
        prevModel->dataRemoved.clearAllOutgoingConnectionsWith(this);
    }
    if (model) {
        connect(model->dataRemoved, [&] { mSelectionModel.clear(); });
    }
    mForEachUI->setModel(std::move(model));
}

void AListView::handleMousePressed(AListItem* item) {
    if (!(AInput::isKeyDown(AInput::LCONTROL) || AInput::isKeyDown(AInput::RCONTROL)) || !mAllowMultipleSelection) {
        clearSelectionInternal();
    }

    auto index = AListModelIndex(mForEachUI->getViews().indexOf(aui::ptr::fake(item)).valueOr(0));
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
        _cast<AListItem>(mForEachUI->getViews()[s.getRow()])->setSelected(false);
    }

    mSelectionModel.clear();
}

void AListView::handleMouseDoubleClicked(AListItem* item) { emit itemDoubleClicked(mForEachUI->getViews().indexOf(aui::ptr::fake(item)).valueOr(0)); }

void AListView::updateSelectionOnItem(size_t i, AListView::SelectAction action) {
    switch (action) {
        case SelectAction::CLEAR_SELECTION_AND_SET:
            clearSelectionInternal();
            mSelectionModel = {AListModelIndex(i)};
            _cast<AListItem>(mForEachUI->getViews()[i])->setSelected(true);
            break;
        case SelectAction::SET:
            if (mAllowMultipleSelection) {
                mSelectionModel << AListModelIndex(i);
            } else {
                clearSelectionInternal();
                mSelectionModel = {AListModelIndex(i)};
            }
            _cast<AListItem>(mForEachUI->getViews()[i])->setSelected(true);
            break;
        case SelectAction::UNSET:
            mSelectionModel.erase(i);
            _cast<AListItem>(mForEachUI->getViews()[i])->setSelected(false);
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