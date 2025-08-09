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
#include "ACustomWindow.h"
#include <AUI/View/AButton.h>
#include <AUI/View/ASpacerExpanding.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "CustomCaptionWindowImplWin32.h"

using CustomCaptionWindowImplCurrent = CustomCaptionWindowImplWin32;


/**
 * @brief Represents a window with customizable caption bar.
 * @ingroup views
 * @details
 * ACustomCaptionWindow implements a fully custom window with customizable caption (title) bar. The way it's done is
 * platform dependent. The goal is to implement a customizable caption bar yet preserve native system caption bar look
 * and feel.
 *
 * Use getCaptionContainer to fill up window caption.
 *
 * Use getContentContainer to fill up the window contents.
 *
 * Do not use setContents/setLayout.
 *
 * Depending on the platform, AUI provides following ASS classes to customize:
 * - ".window-title" for whole window caption container. It has BackgroundSolid by default.
 * - ".title" for window title label
 * - ".window-title-content" for your contents
 * - ".minimize" for minimize button
 * - ".close" for close button
 * - ".middle" for maximize button
 *
 *
 * @specificto{windows}
 * Since Windows does not provide APIs to the customize caption, AUI implements and renders caption by itself, including
 * window icon, title and buttons.
 */
class API_AUI_VIEWS ACustomCaptionWindow : public ACustomWindow, private CustomCaptionWindowImplCurrent {
public:
    ACustomCaptionWindow(const AString& name, int width, int height, bool stacked = false);

    ACustomCaptionWindow() : ACustomCaptionWindow("Custom Caption Window", 240, 124) {}

    ~ACustomCaptionWindow() override = default;

    /**
     * @return customizable container inside caption.
     */
    [[nodiscard]]
    const _<AViewContainer>& getCaptionContainer() const
    {
        return mCaptionContainer;
    }

    /**
     * @return actual window contents.
     */
    [[nodiscard]]
    const _<AViewContainer>& getContentContainer() const
    {
        return mContentContainer;
    }

protected:
    bool isCustomCaptionMaximized() override;

private:
    // these functions are hidden; use ACustomCaptionWindow::getContentContainer() instead
    using ACustomWindow::setLayout;
    using ACustomWindow::setViews;
    using ACustomWindow::addView;
    using ACustomWindow::addViews;
    using ACustomWindow::removeAllViews;
    using ACustomWindow::removeView;
    using ACustomWindow::setContents;

};
