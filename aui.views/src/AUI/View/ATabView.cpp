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
        _new<ASpacerExpanding>(),
    });
    addAssName(".tabview_row_wrap");
}

void ATabButtonRow::setCurrent(size_t i, bool current) {
    _cast<ATabButtonView>(getContents()->getViews()[i])->setCurrent(current);
}

void ATabButtonRow::addTab(const AString &name) {

}
