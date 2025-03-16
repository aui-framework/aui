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

#include <AUI/Model/AListModelIndex.h>
#include <AUI/Model/AListModelSelection.h>
#include <AUI/Model/IListModel.h>
#include <AUI/Layout/AVerticalLayout.h>

#include "AScrollArea.h"
#include "AForEachUI.h"

class AListItem;

/**
 * @brief Displays a list model of strings.
 * @ingroup useful_views
 */
class API_AUI_VIEWS AListView : public AScrollArea {
    friend class AListItem;

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
    explicit AListView(_<IListModel<AString>> model);
    virtual ~AListView();

    void setModel(_<IListModel<AString>> model);

    void updateSelectionOnItem(size_t i, AListView::SelectAction action);

    /**
     * @brief Acts on the item at index i as if the user were left-clicked without keyboard modifiers on it.
     */
    void selectItem(size_t i) { updateSelectionOnItem(i, SelectAction::CLEAR_SELECTION_AND_SET); }

    int getContentFullHeight() { return getLayout()->getMinimumHeight() + 8; }

    void setAllowMultipleSelection(bool allowMultipleSelection);

    [[nodiscard]] AListModelSelection<AString> getSelectionModel() const {
        return AListModelSelection<AString>(mSelectionModel, mForEachUI->model());
    }

signals:
    emits<AListModelSelection<AString>> selectionChanged;
    emits<unsigned> itemDoubleClicked;

    void clearSelection();


private:
    using ForEachUI = AForEachUI<AString, AVerticalLayout>;
    _<ForEachUI> mForEachUI;

    ASet<AListModelIndex> mSelectionModel;
    bool mAllowMultipleSelection = false;

    void handleMousePressed(AListItem* item);
    void handleMouseDoubleClicked(AListItem* item);

    void clearSelectionInternal();
};
