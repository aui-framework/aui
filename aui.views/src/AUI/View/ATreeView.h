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

//
// Created by alex2 on 7/1/2021.
//


#pragma once


#include "AUI/Model/ATreeModelIndex.h"
#include "AViewContainer.h"
#include "AScrollbar.h"
#include <AUI/Model/ITreeModel.h>

/**
 * @brief Displays a tree model of strings.
 *
 * ![](imgs/views/ATreeView.png)
 *
 * @ingroup views_containment
 * @details
 * ATreeView provides view of string-capable ITreeModel objects.
 */
class API_AUI_VIEWS ATreeView: public AViewContainerBase {
private:
    class ContainerView;
    class ItemView;

public:
    ATreeView();
    ATreeView(const AArc<ITreeModel<AString>>& model);
    void setModel(const AArc<ITreeModel<AString>>& model);
    void onScroll(const AScrollEvent& event) override;
    void setSize(glm::ivec2 size) override;
    int getContentMinimumHeight() override;
    void handleMouseMove(ItemView* pView);

    void setViewFactory(const std::function<AArc<AView>(const AArc<ITreeModel<AString>>&, const ATreeModelIndex&)>& viewFactory) {
        mViewFactory = viewFactory;
    }

    void select(const ATreeModelIndex& indexToSelect);

signals:
    emits<ATreeModelIndex> itemSelected;
    emits<ATreeModelIndex> itemMouseClicked;
    emits<ATreeModelIndex> itemMouseDoubleClicked;
    emits<ATreeModelIndex> itemMouseHover;

private:
    AArc<ITreeModel<AString>> mModel;
    AArc<ContainerView> mContent;
    AArc<AScrollbar> mScrollbar;
    AWeakArc<ItemView> mPrevSelection;

    std::function<AArc<AView>(const AArc<ITreeModel<AString>>&, const ATreeModelIndex& index)> mViewFactory;


    void updateScrollbarDimensions();
    void handleMousePressed(ItemView* v);
    void handleMouseDoubleClicked(ItemView* v);
    void handleSelected(ItemView* v);

    void fillViewsRecursively(const AArc<AViewContainer>& content, const ATreeModelIndexOrRoot& index);
    void makeElement(const AArc<AViewContainer>& container, const ATreeModelIndex& childIndex, bool isGroup, const AArc<ATreeView::ItemView>& itemView);
    AArc<ItemView> indexToView(const ATreeModelIndex& target);
};


