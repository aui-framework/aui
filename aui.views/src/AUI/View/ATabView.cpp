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
// Created by alex2 on 24.10.2020.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include "ATabView.h"

using namespace declarative;

ATabView::ATabView() {
    setLayout(std::make_unique<AVerticalLayout>());

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
