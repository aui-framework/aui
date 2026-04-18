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

#include "ACustomCaptionWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>

ACustomCaptionWindow::ACustomCaptionWindow(const AString& name, int width, int height, bool stacked, AWindow* parent, WindowStyle ws)
  : ACustomWindow(name, width, height, parent), CustomCaptionWindowImplCurrent() {
    if (ws != WindowStyle::DEFAULT) {
        setWindowStyle(windowStyle() | ws);
    }
    initCustomCaption(name, stacked, this);

    connect(minimized, this, [&]() { updateMiddleButtonIcon(); });
    connect(restored, this, [&]() { updateMiddleButtonIcon(); });
    connect(maximized, this, [&]() { updateMiddleButtonIcon(); });

    if (mMiddleButton) {
        connect(mMiddleButton->clickedButton, this, [&]() {
            if (isMaximized()) {
                restore();
            } else {
                maximize();
            }
        });
    }

    if (mCloseButton) {
        connect(mCloseButton->clickedButton, this, &AWindow::quit);
    }

    if (mMinimizeButton) {
        connect(mMinimizeButton->clickedButton, this, &AWindow::minimize);
    }
}

bool ACustomCaptionWindow::isCustomCaptionMaximized() { return isMaximized(); }
