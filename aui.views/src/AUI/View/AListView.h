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

#pragma once

#include <AUI/Model/AListModelSelection.h>
#include <AUI/Model/AListModelObserver.h>
#include "AUI/View/AScrollArea.h"
#include "AUI/Model/AListModelIndex.h"
#include "AUI/Model/IListModel.h"
#include "AScrollbar.h"

class AListItem;
class AListViewContainer;

/**
 * @brief Displays a list model of strings.
 * @ingroup useful_views
 */
class API_AUI_VIEWS AListView: public AScrollArea, public AListModelObserver<AString>::IListModelListener
{
    friend class AListItem;
private:
    _<AListViewContainer> mContent;
	ASet<AListModelIndex> mSelectionModel;
	_<AListModelObserver<AString>> mObserver;
    bool mAllowMultipleSelection = false;

    void handleMousePressed(AListItem* item);
    void handleMouseDoubleClicked(AListItem* item);
	
public:
    AListView(): AListView(nullptr) {}
	explicit AListView(const _<IListModel<AString>>& model);
    virtual ~AListView();

    void setModel(const _<IListModel<AString>>& model);

    void selectItem(size_t i);

    int getContentFullHeight() {
        return getLayout()->getMinimumHeight() + 8;
    }

    void setAllowMultipleSelection(bool allowMultipleSelection);

    [[nodiscard]] AListModelSelection<AString> getSelectionModel() const {
	    return AListModelSelection<AString>(mSelectionModel, mObserver->getModel());
	}

    void insertItem(size_t at, const AString& value) override;
    void updateItem(size_t at, const AString& value) override;
    void removeItem(size_t at) override;

    void onDataCountChanged() override;
    void onDataChanged() override;

signals:
	emits<AListModelSelection<AString>> selectionChanged;
	emits<unsigned> itemDoubleClicked;

    void clearSelection();

    bool onGesture(const glm::ivec2 &origin, const AGestureEvent &event) override;
};
