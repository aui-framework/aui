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
#include "AUI/Platform/AWindow.h"
#include "AUI/Enum/WindowStyle.h"
#include "AUI/Enum/Visibility.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Common/Plugin.h>
#include <AUI/ASS/Property/BackgroundImage.h>

#include <windows.h>

namespace {
bool containsPoint(const _<AButton>& button, int px, int py) {
    if (!button) return false;
    if (button->getVisibility() == Visibility::GONE) return false;
    const auto topLeft = button->getPositionInWindow();
    const auto size = button->getSize();
    return px >= topLeft.x && py >= topLeft.y &&
           px < topLeft.x + size.x && py < topLeft.y + size.y;
}

constexpr auto kNcHoverClass = ".nc-hover";
}

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
        to->addView(declarative::Vertical {
                                                    caption,
                                                    _new<ASpacerExpanding>(),
                                            } AUI_LET { it->setExpanding({1, 1}); });
    } else {
        to->setLayout(std::make_unique<AVerticalLayout>());
        to->addView(caption);
        to->addView(mContentContainer = _new<AViewContainer>());
    }
    mContentContainer->setExpanding({1, 1});

    // Hide the minimize/maximize buttons when the window style opts out of them.
    if (auto* window = dynamic_cast<AWindow*>(to)) {
        if (!!(window->windowStyle() & WindowStyle::NO_MINIMIZE_MAXIMIZE)) {
            mMinimizeButton->setVisibility(Visibility::GONE);
            mMiddleButton->setVisibility(Visibility::GONE);
        }
    }

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

AOptional<std::int32_t> CustomCaptionWindowImplWin32::hitTestCaptionButton(int clientPxX, int clientPxY) const {
    if (containsPoint(mMinimizeButton, clientPxX, clientPxY)) return HTMINBUTTON;
    if (containsPoint(mMiddleButton, clientPxX, clientPxY))   return HTMAXBUTTON;
    if (containsPoint(mCloseButton, clientPxX, clientPxY))    return HTCLOSE;
    return std::nullopt;
}

void CustomCaptionWindowImplWin32::updateCaptionButtonNcHover(std::int32_t hitCode) {
    auto applyHover = [&](const _<AButton>& button, bool active) {
        if (!button) return;
        // Toggle the ASS class without disturbing other names the button already carries.
        const auto& names = button->getAssNames();
        const bool hasClass = names.contains(kNcHoverClass);
        if (active && !hasClass) {
            button->addAssName(kNcHoverClass);
        } else if (!active && hasClass) {
            button->removeAssName(kNcHoverClass);
        }
    };
    applyHover(mMinimizeButton, hitCode == HTMINBUTTON);
    applyHover(mMiddleButton,   hitCode == HTMAXBUTTON);
    applyHover(mCloseButton,    hitCode == HTCLOSE);
}
