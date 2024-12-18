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
// Created by alex2 on 27.10.2020.
//

#include "APageSwitch.h"

APageSwitch::APageSwitch(const AString& text, unsigned pageNumber, const _<APageView>& pageView):
    ALabel(text),
    mPageNumber(pageNumber),
    mPageView(pageView)
{

    connect(pageView->pageChanged, this, [&]() {
        emit customCssPropertyChanged();
    });
    connect(clicked, this, [pageView, pageNumber]() {
        pageView->setPageId(pageNumber);
    });
}

int APageSwitch::getContentMinimumWidth() {
    return ALabel::getContentMinimumWidth();
}
