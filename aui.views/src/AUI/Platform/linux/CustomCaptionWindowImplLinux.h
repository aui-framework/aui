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

/**
 * @brief Linux/X11 caption implementation — full client-side decorations.
 *
 * X11 Motif hints cannot reliably preserve individual system buttons across compositors, so
 * we take the Electron/VS Code approach: draw all caption chrome ourselves. The three buttons
 * (minimize, maximize/restore, close) are AUI widgets styled via ASS, and dragging the caption
 * is wired to the `_NET_WM_MOVERESIZE` X11 client-message in ACustomWindowImpl.
 */
class API_AUI_VIEWS CustomCaptionWindowImplLinux {
protected:
    _<AViewContainer> mCaptionContainer;
    _<AViewContainer> mContentContainer;
    _<AButton> mMinimizeButton;
    _<AButton> mMiddleButton;
    _<AButton> mCloseButton;

    void updateMiddleButtonIcon();
    void initCustomCaption(const AString& name, bool stacked, AViewContainer* to);

    virtual bool isCustomCaptionMaximized() = 0;

public:
    CustomCaptionWindowImplLinux() = default;
    virtual ~CustomCaptionWindowImplLinux() = default;
};
