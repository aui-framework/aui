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
// Created by alex2 on 24.10.2020.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include "ATabView.h"


ATabView::ATabView() {
    setLayout(_new<AVerticalLayout>());

    addView(_container<AHorizontalLayout>({
        mButtonsRow = _container<AHorizontalLayout>({}),
        _new<ASpacer>(),
    }));
    addView(mPageView = _new<APageView>());

    mButtonsRow->addAssName(".tabview_row");

    connect(mPageView->pageChanging, this, [&](unsigned pageId) {
        _cast<ATabButtonView>(mButtonsRow->getViews()[mPageView->getPageId()])->setCurrent(false);
        _cast<ATabButtonView>(mButtonsRow->getViews()[pageId])->setCurrent(true);
    });
}

void ATabView::addTab(const _<AView>& view, const AString& name) {
    mPageView->addPage(view);
    auto id = mButtonsRow->getViews().size();
    auto c = _new<ATabButtonView>(name);
    c->setCurrent(id == mPageView->getPageId());
    mButtonsRow->addView(c);

    connect(c->clicked, this, [&, id]() {
       setTabId(id);
    });
}

void ATabView::setTabId(unsigned int tabId) {
    mPageView->setPageId(tabId);
}

void ATabButtonView::getCustomCssAttributes(AMap<AString, AVariant>& map) {
    AView::getCustomCssAttributes(map);
    if (mCurrent)
        map["current"] = true;
}

void ATabButtonView::setCurrent(bool current) {
    mCurrent = current;
    emit customCssPropertyChanged;
}
