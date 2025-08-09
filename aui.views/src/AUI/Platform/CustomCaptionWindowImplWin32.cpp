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
// Created by alex2 on 05.12.2020.
//

#include "CustomCaptionWindowImplWin32.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Common/Plugin.h>
#include <AUI/ASS/Property/BackgroundImage.h>

void CustomCaptionWindowImplWin32::initCustomCaption(const AString& name, bool stacked, AViewContainer* to) {
    auto caption = _new<AViewContainer>();
    caption->setLayout(std::make_unique<AHorizontalLayout>());
    caption->addAssName(".window-title");
    caption->setExpanding({1, 0});

    auto titleLabel = _new<ALabel>(name) << ".title";
    caption->addView(titleLabel);

    mCaptionContainer = _new<AViewContainer>();
    mCaptionContainer->setLayout(std::make_unique<AHorizontalLayout>());
    mCaptionContainer->setExpanding({1, 0 });
    mCaptionContainer->addAssName(".window-title-content");
    caption->addView(mCaptionContainer);

    mMinimizeButton = _new<AButton>();
    mMinimizeButton->addAssName(".minimize");
    mMinimizeButton->addAssName(".default");
    caption->addView(mMinimizeButton);

    mMiddleButton = _new<AButton>();
    mMiddleButton->addAssName(".middle");
    mMiddleButton->addAssName(".default");

    caption->addView(mMiddleButton);

    mCloseButton = _new<AButton>();
    mCloseButton->addAssName(".close");
    mCloseButton->addAssName(".default");
    caption->addView(mCloseButton);

    if (stacked) {
        to->setLayout(std::make_unique<AStackedLayout>());
        to->addView(mContentContainer = _new<AViewContainer>());
        to->addView(_container<AVerticalLayout>({
                                                    caption,
                                                    _new<ASpacerExpanding>(),
                                            }) AUI_LET { it->setExpanding({1, 1}); });
    } else {
        to->setLayout(std::make_unique<AVerticalLayout>());
        to->addView(caption);
        to->addView(mContentContainer = _new<AViewContainer>());
    }
    mContentContainer->setExpanding({1, 1});

    updateMiddleButtonIcon();
}

void CustomCaptionWindowImplWin32::updateMiddleButtonIcon() {
    if (isCustomCaptionMaximized()) {
        mMiddleButton->setCustomStyle({
            ass::BackgroundImage {":uni/caption/restore.svg", {}, {}, ass::Sizing::CENTER }
        });
    } else {
        mMiddleButton->setCustomStyle({
            ass::BackgroundImage {":uni/caption/maximize.svg", {}, {}, ass::Sizing::CENTER }
        });
    }
}
