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

#pragma once

#include <AUI/Model/AListModelSelection.h>
#include <AUI/Model/AListModelObserver.h>
#include "AViewContainer.h"
#include "AUI/Model/AListModelIndex.h"
#include "AUI/Model/IListModel.h"
#include "AScrollbar.h"

class AListItem;
class AListViewContainer;

/**
 * @brief Displays a list model of strings.
 * @ingroup useful_views
 */
class API_AUI_VIEWS AListView: public AViewContainer, public AListModelObserver<AString>::IListModelListener
{
    friend class AListItem;
private:
    _<AListViewContainer> mContent;
    _<AScrollbar> mScrollbar;
	ASet<AListModelIndex> mSelectionModel;
	_<AListModelObserver<AString>> mObserver;
    bool mAllowMultipleSelection = false;

    void updateScrollbarDimensions();
    void handleMousePressed(AListItem* item);
    void handleMouseDoubleClicked(AListItem* item);
	
public:
    AListView(): AListView(nullptr) {}
	explicit AListView(const _<IListModel<AString>>& model);
    virtual ~AListView();

    void setModel(const _<IListModel<AString>>& model);

    void selectItem(size_t i);

    int getContentMinimumHeight(ALayoutDirection layout) override;
    int getContentFullHeight() {
        return getLayout()->getMinimumHeight() + 8;
    }

    void setAllowMultipleSelection(bool allowMultipleSelection);

    [[nodiscard]] AListModelSelection<AString> getSelectionModel() const {
	    return AListModelSelection<AString>(mSelectionModel, mObserver->getModel());
	}

    void setSize(glm::ivec2 size) override;
    void onScroll(const AScrollEvent& event) override;

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
