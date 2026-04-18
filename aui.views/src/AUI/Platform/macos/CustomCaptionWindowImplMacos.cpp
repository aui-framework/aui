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

#include "CustomCaptionWindowImplMacos.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/Enum/WindowStyle.h"
#include <AUI/View/ASpacerFixed.h>

void CustomCaptionWindowImplMacos::initCustomCaption(const AString& name, bool stacked, AViewContainer* to) {
    auto caption = _new<AViewContainer>();
    caption->setLayout(std::make_unique<AHorizontalLayout>());
    caption->addAssName(".window-title");
    caption->setExpanding({ 1, 0 });
    caption->setFixedSize({ 0, 28_dp });

    caption->addView(_new<ASpacerFixed>(80_dp));

    mCaptionContainer = _new<AViewContainer>();
    mCaptionContainer->setLayout(std::make_unique<AHorizontalLayout>());
    mCaptionContainer->setExpanding({ 1, 0 });
    mCaptionContainer->addAssName(".window-title-content");
    caption->addView(mCaptionContainer);

    if (auto* window = dynamic_cast<AWindow*>(to)) {
        window->setWindowStyle(window->windowStyle() | WindowStyle::NO_TITLEBAR);
    }

    if (stacked) {
        to->setLayout(std::make_unique<AStackedLayout>());
        to->addView(mContentContainer = _new<AViewContainer>());
        to->addView(declarative::Vertical {
            caption,
            _new<ASpacerExpanding>(),
        } AUI_LET { it->setExpanding({ 1, 1 }); });
    } else {
        to->setLayout(std::make_unique<AVerticalLayout>());
        to->addView(caption);
        to->addView(mContentContainer = _new<AViewContainer>());
    }
    mContentContainer->setExpanding({ 1, 1 });
}
