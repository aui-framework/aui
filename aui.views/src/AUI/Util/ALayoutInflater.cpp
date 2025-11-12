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
// Created by Alex2772 on 11/12/2021.
//

#include <AUI/Layout/AStackedLayout.h>
#include "ALayoutInflater.h"
#include <AUI/Platform/ASurface.h>

void ALayoutInflater::inflate(aui::no_escape<AViewContainer> root, const _<AView>& view) {
    root->setLayout(std::make_unique<AStackedLayout>());
    if (view) {
        view->setExpanding();
        root->addView(view);
    }
    root->markMinContentSizeInvalid();
}
