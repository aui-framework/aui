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
// Created by Alex2772 on 12/9/2021.
//

#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/win32/Theme.h>
#include <AUI/Platform/CommonRenderingContext.h>
#include <AUI/Traits/callables.h>
#include <variant>
#include <AUI/Platform/ARenderingContextOptions.h>
#include <AUI/AppInfo.h>
#include <algorithm>
#include <dwmapi.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    auto window = reinterpret_cast<AWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    try {
        if (window)
            return window->winProc(hwnd, uMsg, wParam, lParam);
    } catch (const AException& e) {
        ALogger::err("AUI") << "Uncaught exception in window proc: " << e;
    } catch (const std::exception& e) {
        ALogger::err("AUI") << "Uncaught exception in window proc: " << e.what();
    } catch (...) {
        ALogger::err("AUI") << "Uncaught exception in window proc (unknown type)";
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CommonRenderingContext::init(const Init& init) {

    auto& window = init.window;

    // CREATE WINDOW
    WNDCLASSEX winClass;

    // Generate window class name from app_id with random suffix for uniqueness
    // On Windows, multiple instances need unique class names
    ARandom r;
    for (;;) {
        // Use app_id as base, replacing dots with underscores for Windows compatibility
        AString baseClass = aui::app_info::app_id;
        std::replace(baseClass.begin(), baseClass.end(), '.', '_');
        // Add random suffix to ensure uniqueness per instance
        mWindowClass = baseClass + "_" + AString::number(r.nextInt());
        auto u16windowClass = aui::win32::toWchar(mWindowClass);
        winClass.lpszClassName = u16windowClass.c_str();
        winClass.cbSize = sizeof(WNDCLASSEX);
        winClass.style = CS_HREDRAW | CS_VREDRAW;
        winClass.lpfnWndProc = WindowProc;
        winClass.hInstance = GetModuleHandle(nullptr);
        //winClass.hIcon = LoadIcon(mInst, (LPCTSTR)101);
        //winClass.hIconSm = LoadIcon(mInst, (LPCTSTR)101);
        HICON icon = LoadIcon(winClass.hInstance, L"MAINICON");
        winClass.hIcon = icon;
        winClass.hIconSm = icon;
        winClass.hbrBackground = nullptr;
        winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        winClass.lpszMenuName = u16windowClass.c_str();
        winClass.cbClsExtra = 0;
        winClass.cbWndExtra = 0;
        if (RegisterClassEx(&winClass)) {
            break;
        }
    }

    const auto& windowInit = window.init();
    DWORD style = WS_OVERLAPPEDWINDOW;
    // WS_EX_DLGMODALFRAME hides the window icon in the titlebar; it's set only when the icon is explicitly disabled.
    DWORD exStyle = 0;
    bool dwmFrame = false;
    bool disableClose = false;
    if (init.popup) {
        // a top-level window owned by the parent, placed in screen coordinates (see createOverlappingSurfaceImpl), so
        // it can stick out of the parent. Not WS_CHILD: a child is positioned relative to the parent's client area and
        // clipped by it. It never takes activation from the parent: losing focus closes the parent's overlapping
        // surfaces (ASurface::onFocusLost), the popup itself included.
        style = WS_POPUP;
        exStyle = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
    } else {
        std::visit(
            aui::lambda_overloaded {
              [&](const AWindow::Init::DecorationDefault& d) {
                  if (!d.buttons.minimize) style &= ~DWORD(WS_MINIMIZEBOX);
                  if (!d.buttons.maximize) style &= ~DWORD(WS_MAXIMIZEBOX);
                  disableClose = !d.buttons.close;
                  if (!d.win32Icon) exStyle |= WS_EX_DLGMODALFRAME;
              },
              [&](const AWindow::Init::DecorationExtended&) {
                  // AUI draws the titlebar and the buttons (see ATitlebar); DWM keeps the shadow and rounded corners.
                  exStyle = 0;
                  dwmFrame = true;
              },
              [&](const AWindow::Init::DecorationBorderless& d) {
                  exStyle = 0;
                  if (d.shadow) {
                      dwmFrame = true;
                  } else {
                      style = WS_POPUP;
                  }
              },
            },
            windowInit.decoration);
        if (!windowInit.resizable) {
            style &= ~(DWORD(WS_THICKFRAME) | DWORD(WS_MAXIMIZEBOX));
        }
    }

    auto u16windowClass = aui::win32::toWchar(mWindowClass);
    auto u16windowName = aui::win32::toWchar(init.name);
    window.mHandle = CreateWindowEx(exStyle, u16windowClass.c_str(), u16windowName.c_str(), style,
                             GetSystemMetrics(SM_CXSCREEN) / 2 - init.width / 2,
                             GetSystemMetrics(SM_CYSCREEN) / 2 - init.height / 2, init.width, init.height,
                             init.parent != nullptr ? init.parent->mHandle : nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

    aui::UpdateDarkModeForHWND(window.mHandle);

    mWindowDC = GetDC(window.mHandle);

    SetWindowLongPtr(window.mHandle, GWLP_USERDATA, reinterpret_cast<long long int>(&window));

    window.winProc(window.mHandle, WM_CREATE, 0, 0);

    if (init.popup) {
        // small shadow
        SetClassLong(window.mHandle, GCL_STYLE, GetClassLong(window.mHandle, GCL_STYLE) | CS_DROPSHADOW);
    }
    if (disableClose) {
        EnableMenuItem(GetSystemMenu(window.mHandle, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
    }
    if (dwmFrame) {
        // the whole window is the client area (see WM_NCCALCSIZE); the frame extension keeps the shadow.
        const MARGINS margins = { 0, 0, 1, 0 };
        DwmExtendFrameIntoClientArea(window.mHandle, &margins);
        SetWindowPos(
            window.mHandle, nullptr, 0, 0, 0, 0,
            SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
    }

    RECT clientRect;
    GetClientRect(init.window.mHandle, &clientRect);
    init.window.mSize = {clientRect.right - clientRect.left, clientRect.bottom - clientRect.top};
}

void CommonRenderingContext::destroyNativeWindow(ASurface& window) {
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        ReleaseDC(w->mHandle, mWindowDC);

        DestroyWindow(w->mHandle);
    }
    auto u16windowClass = aui::win32::toWchar(mWindowClass);
    UnregisterClass(u16windowClass.c_str(), GetModuleHandle(nullptr));
}

void CommonRenderingContext::beginPaint(ASurface& window) {
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        if (mSmoothResize) {
            AUI_ASSERT(mPainterDC == nullptr);
            mPainterDC = BeginPaint(w->mHandle, &mPaintstruct);
        }
    }
}

void CommonRenderingContext::endPaint(ASurface& window) {
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        if (mSmoothResize) {
            EndPaint(w->mHandle, &mPaintstruct);
            mPainterDC = nullptr;
        }
    }
}
