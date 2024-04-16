// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 12/9/2021.
//

#include <AUI/Platform/CommonRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include "AUI/Platform/ARenderingContextOptions.h"

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


    ARandom r;
    for (;;) {
        mWindowClass = "AUI-" + AString::number(r.nextInt());
        winClass.lpszClassName = mWindowClass.c_str();
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
        winClass.lpszMenuName = mWindowClass.c_str();
        winClass.cbClsExtra = 0;
        winClass.cbWndExtra = 0;
        if (RegisterClassEx(&winClass)) {
            break;
        }
    }

    DWORD style = WS_OVERLAPPEDWINDOW;

    window.mHandle = CreateWindowEx(WS_EX_DLGMODALFRAME, mWindowClass.c_str(), init.name.c_str(), style,
                             GetSystemMetrics(SM_CXSCREEN) / 2 - init.width / 2,
                             GetSystemMetrics(SM_CYSCREEN) / 2 - init.height / 2, init.width, init.height,
                             init.parent != nullptr ? init.parent->mHandle : nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

    mWindowDC = GetDC(window.mHandle);

    SetWindowLongPtr(window.mHandle, GWLP_USERDATA, reinterpret_cast<long long int>(&window));

    // used for ACustomWindow
    window.winProc(window.mHandle, WM_CREATE, 0, 0);

    if ((init.ws & WindowStyle::MODAL) && init.parent) {
        EnableWindow(init.parent->mHandle, false);
    }

    RECT clientRect;
    GetClientRect(init.window.mHandle, &clientRect);
    init.window.mSize = {clientRect.right - clientRect.left, clientRect.bottom - clientRect.top};

    IRenderingContext::init(init);
}

void CommonRenderingContext::destroyNativeWindow(ABaseWindow& window) {
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        ReleaseDC(w->mHandle, mWindowDC);

        DestroyWindow(w->mHandle);
    }
    UnregisterClass(mWindowClass.c_str(), GetModuleHandle(nullptr));
}

void CommonRenderingContext::beginPaint(ABaseWindow& window) {
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        if (mSmoothResize) {
            AUI_ASSERT(mPainterDC == nullptr);
            mPainterDC = BeginPaint(w->mHandle, &mPaintstruct);
        }
    }
}

void CommonRenderingContext::endPaint(ABaseWindow& window) {
    if (auto w = dynamic_cast<AWindow*>(&window)) {
        if (mSmoothResize) {
            EndPaint(w->mHandle, &mPaintstruct);
            mPainterDC = nullptr;
        }
    }
}
