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

#pragma once

#include <AUI/View/AViewContainer.h>
#include <AUI/View/AButton.h>

class ACustomCaptionWindow;

/**
 * @brief macOS caption implementation — delegates caption controls to AppKit's traffic lights.
 *
 * AppKit draws and manages the close/minimize/zoom buttons natively when the window uses
 * NSWindowStyleMaskFullSizeContentView with a transparent titlebar. This impl therefore does
 * not create any AUI-side buttons; instead it reserves horizontal space on the left of the
 * caption container so user content is not covered by the traffic lights.
 *
 * The reserved width matches Apple HIG guidance for the default traffic-light layout.
 */
class API_AUI_VIEWS CustomCaptionWindowImplMacos {
protected:
    _<AViewContainer> mCaptionContainer;
    _<AViewContainer> mContentContainer;

    // The following members are intentionally null on macOS — traffic lights are native.
    // They exist to preserve the protected contract expected by ACustomCaptionWindow.
    _<AButton> mMinimizeButton;
    _<AButton> mMiddleButton;
    _<AButton> mCloseButton;

    void updateMiddleButtonIcon() {}
    void initCustomCaption(const AString& name, bool stacked, AViewContainer* to);

    virtual bool isCustomCaptionMaximized() = 0;

public:
    CustomCaptionWindowImplMacos() = default;
    virtual ~CustomCaptionWindowImplMacos() = default;
};
