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


#include "AUI/GL/gl.h"
#include "AUI/GL/GLDebug.h"
#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/IRenderer.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Util/ARandom.h"
#include "AUI/GL/State.h"
#include "AUI/Thread/AThread.h"
#include "Ole.h"
#include "Win32Util.h"
#include "AUI/Platform/APlatform.h"
#include <AUI/Platform/AMessageBox.h>
#include <AUI/Platform/AWindowManager.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/ASurface.h>
#include <AUI/Platform/ACustomWindow.h>

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Devtools/DevtoolsPanel.h>
#include <AUI/Util/ALayoutInflater.h>
#include <AUI/GL/OpenGLRenderer.h>

#include <AUI/Util/Cache.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/Platform/win32/AComBase.h>

LRESULT AWindow::winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
#define POS glm::ivec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))

    if (mHandle != hwnd) {
        // TODO: reimplement destroyNativeWindow so throwing an exception in OpenGLRenderingContext::init actually
        // matters
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    static glm::ivec2 lastWindowSize;

    currentWindowStorage() = this;

    /*
    if (uMsg != WM_PAINT) {
        if (!mRedrawFlag) {
            // REMIND VINDOVWS ABOUT MY VINDOW I WANT TO REDRAW!!!
            mRedrawFlag = true;
            flagRedraw();
        }

        //bool ok = wglMakeCurrent(GetDC(mHandle), AWindow::context.hrc);
        //assert(ok);
    }
*/

    switch (uMsg) {
/*
        case WM_SIZING:
            if (!isMinimized()) {
                auto rect = reinterpret_cast<LPRECT>(lParam);
                auto w = rect->right - rect->left;
                auto h = rect->bottom - rect->top;
                wglMakeCurrent(mDC, context.hrc);
                AViewContainer::setSize(w, h);
            }
            return true;
            */

        case WM_CREATE: // used for ACustomWindow
            return 0;

        case WM_USER:
            return 0;

        case WM_SETFOCUS:
            onFocusAcquired();
            return 0;

        case WM_KILLFOCUS:
            onFocusLost();
            return 0;


        case WM_PAINT: {
            AThread::processMessages(); // process thread messages because queue freezes when window is frequently redrawn
            //if (!painter::painting)
            {
                redraw();
            }

            return 0;
        }

        case WM_MENUCHAR: return MNC_CLOSE << 16; // silence message beep

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            if (lParam & (1 << 30)) {
                // autoupdate
                onKeyRepeat(AInput::fromNative(wParam));
            } else {
                onKeyDown(AInput::fromNative(wParam));
            }
            return 0;

        case WM_MOVING: {
            auto r = (LPRECT)lParam;
            emit moving(glm::ivec2{r->left, r->top});
            return 0;
        }

        case WM_KEYUP:
            onKeyUp(AInput::fromNative(wParam));
            return 0;

        case WM_CHAR: {
            onCharEntered(static_cast<char32_t>(wParam));
            return 0;
        }

        case WM_SIZE: {

            if (!isMinimized()) {
                RECT windowRect, clientRect;
                GetWindowRect(mHandle, &windowRect);
                GetClientRect(mHandle, &clientRect);
                AUI_NULLSAFE(mRenderingContext)->beginResize(*this);
                AViewContainer::setSize({LOWORD(lParam), HIWORD(lParam)});

                switch (wParam) {
                    case SIZE_MAXIMIZED:
                        emit maximized();
                        break;
                    case SIZE_MINIMIZED:
                        emit minimized();
                        break;
                    case SIZE_RESTORED:
                        emit restored();
                        break;
                }
                AUI_NULLSAFE(mRenderingContext)->endResize(*this);
            }
            return 0;
        }
        case WM_MOUSEMOVE: {
            onPointerMove(POS, {});

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_LEAVE;
            TrackMouseEvent(&tme);

            return 0;
        }
        case WM_MOUSELEAVE:
            onMouseLeave();
            return 0;

        case WM_SETCURSOR: {
            if (LOWORD(lParam) == HTCLIENT) {
                AUI_NULLSAFE(mCursor)->applyNativeCursor(this);
                return true;
            }
            break;
        }


        case WM_CLOSE:
            onCloseButtonClicked();
            return 0;

        case WM_LBUTTONDOWN:
            if (isPressed(APointerIndex::button(AInput::LBUTTON))) {
                // fix AUI_ASSERT(!mPressed);
                onPointerReleased({
                    .position = POS,
                    .pointerIndex = APointerIndex::button(AInput::LBUTTON)
                });
            }
            onPointerPressed({
                .position = POS,
                .pointerIndex = APointerIndex::button(AInput::LBUTTON)
            });
            SetCapture(mHandle);
            return 0;
        case WM_MOUSEWHEEL: {
            const auto SCROLL = 11_pt * 3.f / 120.f;

            onScroll({
                .origin = mapPosition(POS),
                .delta = {0, -float(GET_WHEEL_DELTA_WPARAM(wParam) * SCROLL) },
            });
            return 0;
        }
        case WM_LBUTTONUP: {
            onPointerReleased({
                .position = POS,
                .pointerIndex = APointerIndex::button(AInput::LBUTTON)
            });
            ReleaseCapture();
            return 0;
        }
        case WM_RBUTTONDOWN:
            onPointerPressed({
                .position = POS,
                .pointerIndex = APointerIndex::button(AInput::RBUTTON)
            });
            SetCapture(mHandle);
            return 0;
        case WM_RBUTTONUP:
            onPointerReleased({
                .position = POS,
                .pointerIndex = APointerIndex::button(AInput::RBUTTON)
            });
            ReleaseCapture();
            return 0;

        case WM_DPICHANGED: {
            auto prevDpi = getDpiRatio();
            updateDpi();
            setSize({getWidth() * getDpiRatio() / prevDpi, getHeight() * getDpiRatio() / prevDpi});
            return 0;
        }

        case WM_GETMINMAXINFO: {
            MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);

            RECT r = {0, 0, getMinimumWidth(), getMinimumHeight()};
            AdjustWindowRectEx(&r, GetWindowLongPtr(mHandle, GWL_STYLE), false, GetWindowLongPtr(mHandle, GWL_EXSTYLE));
            info->ptMinTrackSize.x = r.right - r.left;
            info->ptMinTrackSize.y = r.bottom - r.top;
        }
            return 0;
        case WM_ERASEBKGND:
            return true;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);

#undef GET_X_LPARAM
#undef GET_Y_LPARAM
#undef POS
}


void AWindow::quit() {
    HWND parentHwnd = nullptr;

    bool isModal = !!(mWindowStyle & WindowStyle::MODAL);

    if (isModal && mParentWindow) {
        parentHwnd = mParentWindow->mHandle;
    }

    if (mHandle) {
        ShowWindow(mHandle, SW_HIDE);
        DestroyWindow(mHandle);
        mHandle = nullptr;
    }

    getWindowManager().mWindows.removeFirst(mSelfHolder);
    setLayout(nullptr);

    // Schedule object destruction
    AThread::current()->enqueue([self = std::move(mSelfHolder), parentHwnd, isModal]() mutable noexcept {
        self = nullptr;
        if (parentHwnd && IsWindow(parentHwnd) && isModal) {
            // Parent window should be activated BEFORE child is closed.
            EnableWindow(parentHwnd, true);
            SetActiveWindow(parentHwnd);
            SetForegroundWindow(parentHwnd);
        }
    });
}


AWindow::~AWindow() {
    ShowWindow(mHandle, SW_HIDE);
    mRenderingContext->destroyNativeWindow(*this);
}


void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;
    if (!mHandle) return;
    if (!!(ws & WindowStyle::SYS)) {
        SetWindowLongPtr(mHandle, GWL_STYLE, GetWindowLong(mHandle, GWL_STYLE) & ~(WS_CAPTION | WS_THICKFRAME
            | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU) | WS_CHILD);
        {
            LONG lExStyle = GetWindowLong(mHandle, GWL_EXSTYLE);
            lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
            SetWindowLong(mHandle, GWL_EXSTYLE, lExStyle);
        }
        SetWindowPos(mHandle, NULL, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

        // small shadow
        SetClassLong(mHandle, GCL_STYLE, GetClassLong(mHandle, GCL_STYLE) | CS_DROPSHADOW);
    } else {
        if (!!(ws & WindowStyle::NO_MINIMIZE_MAXIMIZE)) {
            SetWindowLongPtr(mHandle, GWL_STYLE,
                             GetWindowLong(mHandle, GWL_STYLE) & ~(WS_THICKFRAME |
                                     WS_SYSMENU) | WS_CAPTION);
        } else {
            SetWindowLongPtr(mHandle, GWL_STYLE, GetWindowLong(mHandle, GWL_STYLE) | WS_THICKFRAME);
        }

        if (!!(ws & WindowStyle::NO_RESIZE)) {
            SetWindowLongPtr(mHandle, GWL_STYLE,
                             GetWindowLong(mHandle, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW | WS_DLGFRAME |
                                     WS_SYSMENU | WS_CAPTION);
        }
        if (!!(ws & WindowStyle::NO_DECORATORS)) {
            LONG lExStyle = GetWindowLong(mHandle, GWL_EXSTYLE);
            lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
            SetWindowLong(mHandle, GWL_EXSTYLE, lExStyle);
            SetWindowPos(mHandle, NULL, 0, 0, 0, 0,
                         SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }
    }
}


float AWindow::fetchDpiFromSystem() const {
    if (mHandle) {
        typedef UINT(WINAPI* GetDpiForWindow_t)(_In_ HWND);
        static auto GetDpiForWindow = (GetDpiForWindow_t) GetProcAddress(GetModuleHandleA("User32.dll"),
                                                                         "GetDpiForWindow");
        if (GetDpiForWindow) {
            return GetDpiForWindow(mHandle) / 96.f;
        } else {
            return APlatform::getDpiRatio();
        }
    }
    return 1.f;
}

void AWindow::restore() {
    if (mHandle) ShowWindow(mHandle, SW_RESTORE);
}

void AWindow::minimize() {
    if (mHandle) ShowWindow(mHandle, SW_MINIMIZE);
}

bool AWindow::isMinimized() const {
    if (!mHandle) return false;
    return IsIconic(mHandle);
}


bool AWindow::isMaximized() const {
    if (!mHandle) return false;
    return IsZoomed(mHandle);
}

void AWindow::maximize() {
    if (mHandle) ShowWindow(mHandle, SW_MAXIMIZE);
}

glm::ivec2 AWindow::getWindowPosition() const {
    if (!mHandle) return {0, 0};
    RECT r;
    GetWindowRect(mHandle, &r);
    return {r.left, r.top};
}

void AWindow::flagRedraw() {
    if (mRedrawFlag && mHandle) {
        getThread()->enqueue([handle = mHandle] {
            InvalidateRect(handle, nullptr, true);
        });
        mRedrawFlag = false;
    }
}


void AWindow::setSize(glm::ivec2 size) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, size.x, size.y);
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize({width, height});

    if (!mHandle) return;

    RECT r = {0, 0, width, height};
    AdjustWindowRectEx(&r, GetWindowLongPtr(mHandle, GWL_STYLE), false, GetWindowLongPtr(mHandle, GWL_EXSTYLE));
    MoveWindow(mHandle, x, y, r.right - r.left, r.bottom - r.top, false);
}

glm::ivec2 AWindow::mapPosition(const glm::ivec2& position) {
    if (!mHandle) return position;
    POINT p = {position.x, position.y};
    ScreenToClient(mHandle, &p);
    return {p.x, p.y};
}
glm::ivec2 AWindow::unmapPosition(const glm::ivec2& position) {
    if (!mHandle) return position;
    POINT p = {position.x, position.y};
    ClientToScreen(mHandle, &p);
    return {p.x, p.y};
}

void AWindow::show() {
    if (!getWindowManager().mWindows.contains(_cast<AWindow>(aui::ptr::shared_from_this(this)))) {
        getWindowManager().mWindows << _cast<AWindow>(aui::ptr::shared_from_this(this));
    }
    try {
        mSelfHolder = _cast<AWindow>(aui::ptr::shared_from_this(this));
    } catch (...) {
        mSelfHolder = nullptr;
    }
    AThread::current() << [this, self = shared_from_this()]() {
        redraw();
    };

    UpdateWindow(mHandle);
    ShowWindow(mHandle, SW_SHOWNORMAL);

    emit shown();
}
void AWindow::setIcon(const AImage& image) {
    if (!mHandle) return;
    AUI_ASSERT(image.format() & APixelFormat::BYTE);

    if (mIcon) {
        DestroyIcon(mIcon);
    }


    auto bmpColor = aui::win32::imageRgbToBitmap(image);
    auto bmpMask = aui::win32::imageRgbToBitmap(image, aui::win32::BitmapMode::A);

    ICONINFO ii;
    ii.fIcon = TRUE;
    ii.hbmMask = bmpMask;
    ii.hbmColor = bmpColor;
    mIcon = CreateIconIndirect(&ii);

    SendMessage(mHandle, WM_SETICON, ICON_BIG, (LPARAM)mIcon);
}

void AWindow::hide() {
    if (!mHandle) return;
    ShowWindow(mHandle, SW_HIDE);
}


void AWindowManager::notifyProcessMessages() {
    if (!mWindows.empty()) {
        auto& lastWindow = mWindows.back();

        // we don't need to notify MS Windows' message queue about new message if message sent from UI thread.
        if (lastWindow->getThread() != AThread::current()) {
            PostMessage(lastWindow->mHandle, WM_USER, 0, 0);
        }
    }
}


void AWindowManager::loop() {
    MSG msg;
    for (mLoopRunning = true; mLoopRunning && !mWindows.empty();) {
        if (GetMessage(&msg, nullptr, 0, 0) == 0) {
            break;
        }
        mWatchdog.runOperation([&] {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            AThread::processMessages();
        });
    }
}

void AWindow::blockUserInput(bool blockUserInput) {
    EnableWindow(mHandle, !blockUserInput);
}

void AWindow::allowDragNDrop() {
    class DropTarget: public AComBase<DropTarget, IDropTarget> {
    public:
        DropTarget(ASurface* window) : mWindow(window) {}

        HRESULT __stdcall DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override {
            auto effect = DROPEFFECT_NONE;
            mMimed = Ole::toMime(pDataObj);
            if (mWindow->onDragEnter({ mMimed, { pt.x, pt.y } })) {
                if (*pdwEffect & DROPEFFECT_COPY) {
                    effect = DROPEFFECT_COPY;
                } else {
                    effect = DROPEFFECT_MOVE;
                }
            }
            *pdwEffect = mOleEffect = effect;
            return S_OK;
        }

        HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override {
            *pdwEffect = mOleEffect;
            return S_OK;
        }

        HRESULT __stdcall DragLeave() override {
            mWindow->onDragLeave();
            mMimed.clear();
            return S_OK;
        }

        HRESULT __stdcall Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override {
            *pdwEffect = mOleEffect;
            mWindow->onDragDrop({ mMimed, { pt.x, pt.y} });
            return S_OK;
        }

    private:
        AMimedData mMimed;
        ASurface* mWindow;
        DWORD mOleEffect;
    };
    Ole::inst();

    auto r = RegisterDragDrop(mHandle, new DropTarget(this));
    AUI_ASSERT(r == S_OK);
}

void AWindow::showTouchscreenKeyboardImpl() {
    ASurface::showTouchscreenKeyboardImpl();
}

void AWindow::hideTouchscreenKeyboardImpl() {
    ASurface::hideTouchscreenKeyboardImpl();
}

void AWindow::moveToCenter() {
    auto m = MonitorFromWindow(mHandle, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO info;
    info.cbSize = sizeof(info);
    GetMonitorInfo(m, &info);
    glm::ivec2 topLeft = { info.rcMonitor.left, info.rcMonitor.top };
    glm::ivec2 bottomRight = { info.rcMonitor.right, info.rcMonitor.bottom };
    setPosition(topLeft + (bottomRight - topLeft - getSize()) / 2);
}

void AWindow::setMobileScreenOrientation(AScreenOrientation screenOrientation) {

}
