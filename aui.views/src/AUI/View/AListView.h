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

#pragma once

#include <AUI/Model/AListModelIndex.h>
#include <AUI/Model/AListModelObserver.h>
#include <AUI/Model/AListModelSelection.h>
#include <AUI/Model/IListModel.h>

#include "AScrollArea.h"

class AListItem;
class AListViewContainer;

/**
 * @brief Displays a list model of strings.
 * @ingroup useful_views
 */
class API_AUI_VIEWS AListView : public AScrollArea, public AListModelObserver<AString>::IListModelListener {
    friend class AListItem;

   private:
    _<AListViewContainer> mContent;
    ASet<AListModelIndex> mSelectionModel;
    _<AListModelObserver<AString>> mObserver;
    bool mAllowMultipleSelection = false;

    void handleMousePressed(AListItem* item);
    void handleMouseDoubleClicked(AListItem* item);

    void clearSelectionInternal();

   public:
    /**
     * @brief Selection action for updateSelectionOnItem.
     */
    enum class SelectAction {
        /**
         * @brief Clears old selection and selects the specified index. Used by selectItem.
         */
        CLEAR_SELECTION_AND_SET,

        /**
         * @brief Selects the specified index. In single selection mode, acts like CLEAR_SELECTION_AND_SET.
         */
        SET,

        /**
         * @brief Deselects the specified index.
         */
        UNSET,

        /**
         * @brief Selects or deselects the specified index depending on it's current state.
         */
        TOGGLE,
    };

    AListView() : AListView(nullptr) {}
    explicit AListView(const _<IListModel<AString>>& model);
    virtual ~AListView();

    void setModel(const _<IListModel<AString>>& model);

    void updateSelectionOnItem(size_t i, AListView::SelectAction action);

    /**
     * @brief Acts on the item at index i as if the user were left-clicked without keyboard modifiers on it.
     */
    void selectItem(size_t i) { updateSelectionOnItem(i, SelectAction::CLEAR_SELECTION_AND_SET); }

    int getContentFullHeight() { return getLayout()->getMinimumHeight() + 8; }

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

    bool onGesture(const glm::ivec2& origin, const AGestureEvent& event) override;
};
