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
// Created by Alex2772 on 12/5/2021.
//


#include "UIMatcher.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/AWindowManager.h>

ASet<_<AView>> UIMatcher::toSet() const {
    ASet<_<AView>> result;

    for (auto& window : AWindowBase::getWindowManager().getWindows()) {
        processContainer(result, window);
    }
    return result;
}

UIMatcher*& UIMatcher::currentImpl() {
    thread_local UIMatcher* matcher;
    return matcher;
}

void UIMatcher::processContainer(ASet<_<AView>>& destination, const _<AViewContainer>& container) const {
    for (auto& view : container) {
        if (mIncludeInvisibleViews || (view->getVisibility() == Visibility::VISIBLE)) {
            if (mMatcher->matches(view)) {
                destination << view;
            }
            if (auto currentContainer = _cast<AViewContainer>(view)) {
                processContainer(destination, currentContainer);
            }
        }
    }
}
