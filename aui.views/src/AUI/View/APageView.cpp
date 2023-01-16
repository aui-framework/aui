// AUI Framework - Declarative UI toolkit for modern C++17
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
// Created by alex2 on 24.10.2020.
//

#include <AUI/Layout/AStackedLayout.h>
#include "APageView.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/Logging/ALogger.h>

APageView::APageView() {
    setLayout(_new<AStackedLayout>());
}

void APageView::setPageId(unsigned int pageId) {
    assert(pageId < getViews().size());
    emit pageChanging(pageId);
    getViews()[mPageId]->setVisibility(Visibility::GONE);
    getViews()[pageId]->setVisibility(Visibility::VISIBLE);
    mPageId = pageId;
    emit pageChanged();

    AWindow::current()->flagUpdateLayout();
}

void APageView::addPage(const _<AView>& view) {
    view->setVisibility(mPageId == getViews().size() ? Visibility::VISIBLE : Visibility::GONE);
    addView(view);
}
