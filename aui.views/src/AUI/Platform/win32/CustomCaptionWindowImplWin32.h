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

#pragma once

#include <AUI/View/AViewContainer.h>
#include <AUI/View/AButton.h>
#include <AUI/Common/AOptional.h>

#include <cstdint>

class API_AUI_VIEWS CustomCaptionWindowImplWin32 {
protected:
    _<AViewContainer> mCaptionContainer;
    _<AViewContainer> mContentContainer;
    _<AButton> mMinimizeButton; // _
    _<AButton> mMiddleButton; // []
    _<AButton> mCloseButton; // X

    void updateMiddleButtonIcon();
    void initCustomCaption(const AString& name, bool stacked, AViewContainer* to);

    virtual bool isCustomCaptionMaximized() = 0;

public:
    CustomCaptionWindowImplWin32() = default;

    virtual ~CustomCaptionWindowImplWin32() = default;

    /**
     * @brief Test a client-space point against the caption button rects.
     * @param clientPxX client-area x in physical pixels (relative to window top-left).
     * @param clientPxY client-area y in physical pixels.
     * @return HTMINBUTTON / HTMAXBUTTON / HTCLOSE when the point is over the corresponding
     *         button, or std::nullopt when the point is outside all three. Returning an HT
     *         button code from WM_NCHITTEST enables Win11 snap-layout flyouts on hover of
     *         the maximize button and lets the OS dispatch SC_MINIMIZE/SC_MAXIMIZE/SC_CLOSE
     *         automatically on click.
     */
    AOptional<std::int32_t> hitTestCaptionButton(int clientPxX, int clientPxY) const;

    /**
     * @brief Sync button hover visuals from WM_NCMOUSEMOVE.
     *
     * When WM_NCHITTEST returns an HT button code, the region becomes non-client; AButton
     * stops receiving WM_MOUSEMOVE and its own hover state freezes. This method reflects
     * the NC hit result back into the buttons so their rendering tracks the cursor.
     *
     * @param hitCode the HT* code reported by the latest WM_NCMOUSEMOVE, or 0 to clear.
     */
    void updateCaptionButtonNcHover(std::int32_t hitCode);
};


