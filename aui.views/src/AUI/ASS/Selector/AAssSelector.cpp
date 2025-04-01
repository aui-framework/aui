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
// Created by alex2 on 31.12.2020.
//

#include "AAssSelector.h"
#include <AUI/View/AView.h>
#include <AUI/ASS/AAssHelper.h>

bool ass::IAssSubSelector::isStateApplicable(AView* view) {
    return true;
}

void ass::IAssSubSelector::setupConnections(AView* view, const _<AAssHelper>& helper) {
}

void ass::AAssSelector::setupConnections(AView* view, const _<AAssHelper>& helper) const {
    for (const auto& s : mSubSelectors) {
        if (s->isPossiblyApplicable(view)) {
            s->setupConnections(view, helper);
            break;
        }
    }
}
