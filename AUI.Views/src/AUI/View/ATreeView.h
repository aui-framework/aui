/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 7/1/2021.
//


#pragma once


#include "AViewContainer.h"
#include "AScrollbar.h"
#include <AUI/Model/ITreeModel.h>

class API_AUI_VIEWS ATreeView: public AViewContainer {
private:
    class ContainerView;
    class ItemView;

    _<ITreeModel<AString>> mModel;
    _<ContainerView> mContent;
    _<AScrollbar> mScrollbar;

    std::function<_<AView>(const _<ITreeModel<AString>>&, const ATreeIndex& index)> mViewFactory;


    void updateScrollbarDimensions();
    void handleMousePressed(ItemView* v);
    void handleMouseDoubleClicked(ItemView* v);

    void fillViewsRecursively(const _<AViewContainer>& content, const ATreeIndex& index);
    void makeElement(const _<AViewContainer>& container, const ATreeIndex& childIndex, bool isGroup, const _<ATreeView::ItemView>& itemView);
public:
    ATreeView();
    ATreeView(const _<ITreeModel<AString>>& model);
    void setModel(const _<ITreeModel<AString>>& model);
    void onMouseWheel(glm::ivec2 pos, int delta) override;
    void setSize(int width, int height) override;
    int getContentMinimumHeight() override;
    void handleMouseMove(ItemView* pView);

    void setViewFactory(const std::function<_<AView>(const _<ITreeModel<AString>>&, const ATreeIndex&)>& viewFactory) {
        mViewFactory = viewFactory;
    }

signals:
    emits<ATreeIndex> itemMouseHover;
};


