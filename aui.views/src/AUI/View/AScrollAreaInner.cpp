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

#include "AScrollAreaInner.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/View/AView.h"

void AScrollAreaInner::setContents(_<AView> content) {
    if (mContents) {
        removeView(mContents);
    }
    content->setPosition({0, 0});
    if (content) addView(content);
    mContents = std::move(content);
    updateContentsScroll();
}

void AScrollAreaInner::updateLayout() {
    AViewContainer::updateLayout();
    AUI_NULLSAFE(mContents)->setSize(glm::max(mContents->getMinimumSize(), getSize()));
}
void AScrollAreaInner::updateContentsScroll() {
    AUI_NULLSAFE(mContents)->setPosition(-glm::ivec2(mScroll));
}