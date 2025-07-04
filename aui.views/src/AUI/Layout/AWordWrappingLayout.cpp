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
// Created by Alex2772 on 9/7/2021.
//

#include <AUI/Util/AWordWrappingEngineImpl.h>
#include "AWordWrappingLayout.h"


void AWordWrappingLayout::addView(const _<AView>& view, AOptional<size_t> index) {
    ALinearLayout::addView(view, index);

    if (index) {
        if (mViewEntry.size() <= *index) {
            mViewEntry.resize(*index + 1);
        }
        mViewEntry[*index] = AViewEntry{ view };
    } else {
        mViewEntry << AViewEntry{ view };
    }
}

void AWordWrappingLayout::removeView(aui::no_escape<AView> view, size_t index) {
    ALinearLayout::removeView(view, index);

    mViewEntry.removeAt(index);
}



int AWordWrappingLayout::getMinimumWidth() {
    return 0;
}

int AWordWrappingLayout::getMinimumHeight() {
    int m = 0;
    for (auto& view : mViews) {
        m = (glm::max)(view->getPosition().y + view->getSize().y, m);
    }
    return m;
}

void AWordWrappingLayout::onResize(int x, int y, int width, int height) {
    AVector<_<AWordWrappingEngineBase::Entry>> entries;
    for (auto& v : mViewEntry) {
        entries << aui::ptr::fake_shared(&v);
    }
    AWordWrappingEngine we;
    we.setEntries(std::move(entries));
    we.performLayout({x, y}, {width, height});
}