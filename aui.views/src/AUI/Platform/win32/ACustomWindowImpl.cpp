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

#include "AUI/Platform/ACustomWindow.h"
#include "AUI/Platform/ADesktop.h"
#include <cstring>
#include <AUI/View/AButton.h>

#include <glm/gtc/matrix_transform.hpp>

#include "AUI/Render/IRenderer.h"
#include "AUI/Common/AColor.h"
#include "AUI/Platform/AFontManager.h"

#include <dwmapi.h>
#include <AUI/Util/kAUI.h>


LRESULT ACustomWindow::winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
    switch (uMsg)
    {
        case WM_CREATE: {
            const MARGINS shadow = {1, 1, 1, 1};
            DwmExtendFrameIntoClientArea((HWND) getNativeHandle(), &shadow);

            // update window size
            RECT rcClient;
            GetWindowRect(getNativeHandle(), &rcClient);

            SetWindowPos(getNativeHandle(),
                         NULL,
                         rcClient.left, rcClient.top,
                         rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                         SWP_FRAMECHANGED);
            return 0;
        }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);

        info->ptMinTrackSize.x = getMinimumWidth();
        info->ptMinTrackSize.y = getMinimumHeight();
        return 0;
    }
    case WM_NCCALCSIZE:
        return 0;
    case WM_MOVING:
        if (!mDragging) {
            mDragging = true;
            auto info = reinterpret_cast<RECT*>(lParam);
            emit dragBegin({info->left, info->top });
        }
        break;
    case WM_EXITSIZEMOVE:
        if (mDragging) {
            mDragging = false;
            emit dragEnd();
        }
        return 0;
    case WM_NCHITTEST:
    {
        unsigned result = 0;
        const LONG border_width = 8; //in pixels
        RECT winrect;
        GetWindowRect(getNativeHandle(), &winrect);

        auto x = GET_X_LPARAM(lParam);
        auto y = GET_Y_LPARAM(lParam);

        bool resizeWidth = !(mWindowStyle & WindowStyle::NO_RESIZE);//window->minimumWidth() != window->maximumWidth();
        bool resizeHeight = resizeWidth;//window->minimumHeight() != window->maximumHeight();

        if (resizeWidth)
        {
            //left border
            if (x >= winrect.left && x < winrect.left + border_width)
            {
                result = HTLEFT;
            }
            //right border
            if (x < winrect.right && x >= winrect.right - border_width)
            {
                result = HTRIGHT;
            }
        }
        if (resizeHeight)
        {
            //bottom border
            if (y < winrect.bottom && y >= winrect.bottom - border_width)
            {
                result = HTBOTTOM;
            }
            //top border
            if (y >= winrect.top && y < winrect.top + border_width)
            {
                result = HTTOP;
            }
        }
        if (resizeWidth && resizeHeight)
        {
            //bottom left corner
            if (x >= winrect.left && x < winrect.left + border_width &&
                y < winrect.bottom && y >= winrect.bottom - border_width)
            {
                result = HTBOTTOMLEFT;
            }
            //bottom right corner
            if (x < winrect.right && x >= winrect.right - 22 &&
                y < winrect.bottom && y >= winrect.bottom - 12)
            {
                result = HTBOTTOMRIGHT;
            }
            //top left corner
            if (x >= winrect.left && x < winrect.left + border_width &&
                y >= winrect.top && y < winrect.top + border_width)
            {
                result = HTTOPLEFT;
            }
            //top right corner
            if (x < winrect.right && x >= winrect.right - border_width &&
                y >= winrect.top && y < winrect.top + border_width)
            {
                result = HTTOPRIGHT;
            }
        }

        //TODO: allow move?
        if (!result) {
            if (isCaptionAt({x - winrect.left, y - winrect.top})) {
                result = HTCAPTION;
            }
        }
        if (result)
            return result;
    } //end case WM_NCHITTEST

    }
    return AWindow::winProc(hwnd, uMsg, wParam, lParam);

#undef GET_X_LPARAM
#undef GET_Y_LPARAM
}

void ACustomWindow::doDrawWindow()
{
    // Frame
    AWindow::doDrawWindow();
}

ACustomWindow::ACustomWindow(const AString& name, int width, int height, AWindow* parent): AWindow(nullptr)
{
    // init here to be sure vtable for wndProc for WM_CREATE is initialized
    windowNativePreInit(name, width, height, parent, WindowStyle::DEFAULT);
}

void ACustomWindow::setSize(glm::ivec2 size)
{
    AViewContainer::setSize(size);
    auto pos = getWindowPosition();

    MoveWindow(mHandle, pos.x, pos.y, size.x, size.y, false);
}

bool ACustomWindow::isCaptionAt(const glm::ivec2& pos) {
    if (pos.y <= mTitleHeight) {
        if (auto v = getViewAtRecursive(pos)) {
            if (!(_cast<AButton>(v)) &&
                !v->getAssNames().contains(".override-title-dragging")) {
                return true;
            }
        }
    }
    return false;
}

void ACustomWindow::onPointerPressed(const APointerPressedEvent& event) {
    AWindow::onPointerPressed(event);
}

void ACustomWindow::onPointerReleased(const APointerReleasedEvent& event) {
    AWindow::onPointerReleased(event);
}
