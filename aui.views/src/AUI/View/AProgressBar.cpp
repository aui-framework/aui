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
// Created by Alex2772 on 1/8/2022.
//

#include "AProgressBar.h"

AProgressBar::AProgressBar() : mInner(_new<Inner>()) {
    addView(mInner);
    mInner->setPosition({0, 0});
}

AProgressBar::Inner::~Inner() {

}

AProgressBar::~AProgressBar() {

}

void AProgressBar::setSize(glm::ivec2 size) {
    AViewContainerBase::setSize(size);
    updateInnerWidth();
}

void AProgressBar::updateInnerWidth() {
    mInner->setGeometry(mPadding.left,
                        mPadding.top,
                        int(mValue * float(getContentWidth() - mPadding.horizontal())),
                        getContentHeight() - mPadding.vertical());
}

_<AView> declarative::ProgressBar::operator()() {
    auto view = _new<AProgressBar>();
    progress.bindTo(view->value().assignment());
    return view;
}
