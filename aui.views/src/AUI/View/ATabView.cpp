/*
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 24.10.2020.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include "ATabView.h"


ATabView::ATabView() {
    setLayout(_new<AVerticalLayout>());

    addView(mRow = _new<ATabButtonRow>());
    addView(mPageView = _new<APageView>());
    mPageView->setExpanding();
    connect(mPageView->pageChanging, this, [&](unsigned pageId) {
        mRow->setCurrent(mPageView->getPageId(), false);
        mRow->setCurrent(pageId, true);
    });
}

void ATabView::addTab(const _<AView>& view, const AString& name) {
    mPageView->addPage(view);
    view->setExpanding();
    auto id = mRow->getContents()->getViews().size();
    auto c = _new<ATabButtonView>(name);
    c->setCurrent(id == mPageView->getPageId());
    mRow->getContents()->addView(c);

    connect(c->clicked, this, [&, id]() {
       setTabId(id);
    });
}

void ATabView::setTabId(unsigned int tabId) {
    mPageView->setPageId(tabId);
}

void ATabButtonView::setCurrent(bool current) {
    mCurrent = current;
    emit customCssPropertyChanged;
}

bool ATabButtonView::selectableIsSelectedImpl() {
    return mCurrent;
}

ATabButtonRow::ATabButtonRow() {
    setContents(Horizontal {
        mContents = _container<AHorizontalLayout>({}) << ".tabview_row",
        _new<ASpacer>(),
    });
    addAssName(".tabview_row_wrap");
}

void ATabButtonRow::setCurrent(size_t i, bool current) {
    _cast<ATabButtonView>(getContents()->getViews()[i])->setCurrent(current);
}

void ATabButtonRow::addTab(const AString &name) {

}
