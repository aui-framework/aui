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


#pragma once


#include "AUI/Model/ATreeModelIndex.h"
#include "AViewContainer.h"
#include "AScrollbar.h"
#include <AUI/Model/ITreeModel.h>

/**
 * @brief Displays a tree model of strings.
 * @ingroup useful_views
 */
class API_AUI_VIEWS ATreeView: public AViewContainer {
private:
    class ContainerView;
    class ItemView;

public:
    ATreeView();
    ATreeView(const _<ITreeModel<AString>>& model);
    void setModel(const _<ITreeModel<AString>>& model);
    void onScroll(const AScrollEvent& event) override;
    void setSize(glm::ivec2 size) override;
    int getContentMinimumHeight(ALayoutDirection layout) override;
    void handleMouseMove(ItemView* pView);

    void setViewFactory(const std::function<_<AView>(const _<ITreeModel<AString>>&, const ATreeModelIndex&)>& viewFactory) {
        mViewFactory = viewFactory;
    }

    void select(const ATreeModelIndex& indexToSelect);

signals:
    emits<ATreeModelIndex> itemSelected;
    emits<ATreeModelIndex> itemMouseClicked;
    emits<ATreeModelIndex> itemMouseDoubleClicked;
    emits<ATreeModelIndex> itemMouseHover;

private:
    _<ITreeModel<AString>> mModel;
    _<ContainerView> mContent;
    _<AScrollbar> mScrollbar;
    _weak<ItemView> mPrevSelection;

    std::function<_<AView>(const _<ITreeModel<AString>>&, const ATreeModelIndex& index)> mViewFactory;


    void updateScrollbarDimensions();
    void handleMousePressed(ItemView* v);
    void handleMouseDoubleClicked(ItemView* v);
    void handleSelected(ItemView* v);

    void fillViewsRecursively(const _<AViewContainer>& content, const ATreeModelIndex& index);
    void makeElement(const _<AViewContainer>& container, const ATreeModelIndex& childIndex, bool isGroup, const _<ATreeView::ItemView>& itemView);
    _<ItemView> indexToView(const ATreeModelIndex& target);
};


