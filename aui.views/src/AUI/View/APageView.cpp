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
    AUI_ASSERT(pageId < getViews().size());
    emit pageChanging(pageId);
    getViews()[mPageId]->setVisibility(Visibility::GONE);
    getViews()[pageId]->setVisibility(Visibility::VISIBLE);
    mPageId = pageId;
    emit pageChanged();
}

void APageView::addPage(const _<AView>& view) {
    view->setVisibility(mPageId == getViews().size() ? Visibility::VISIBLE : Visibility::GONE);
    addView(view);
}
