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
#include "AUI/Util/AImageDrawable.h"
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
#include <AUI/View/ATitlebar.h>

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
#include <AUI/Platform/win32/Theme.h>
#include <dwmapi.h>

namespace {
/**
 * @return true if AUI draws the titlebar and handles the window frame (see AWindow::Init::DecorationExtended and
 *         AWindow::Init::DecorationBorderless).
 */
bool hasCustomFrame(const AWindow& window) {
    return !window.isPopup() && !std::holds_alternative<AWindow::Init::DecorationDefault>(window.init().decoration);
}
}   // namespace

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

        case WM_CREATE:
            updateDpi();
            return 0;

        case WM_NCCALCSIZE: {
            if (!hasCustomFrame(*this) || wParam == FALSE) {
                break;
            }
            // The client area covers the whole window: AUI draws the titlebar and the borders.
            if (IsZoomed(hwnd)) {
                // Maximized windows would otherwise be clipped off-screen by the frame.
                auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                const int frameX = GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
                const int frameY = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
                RECT& rc = params->rgrc[0];
                rc.left += frameX;
                rc.right -= frameX;
                rc.top += frameY;
                rc.bottom -= frameY;
            }
            return 0;
        }

        case WM_NCHITTEST: {
            if (!hasCustomFrame(*this)) {
                break;
            }
            POINT pt { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(hwnd, &pt);
            const glm::ivec2 local { pt.x, pt.y };

            // Window buttons win over the top resize border: otherwise the top pixels of a min/max/close button
            // would be treated as HTTOP and start a resize drag instead of registering a button click. Returning an
            // HT button code enables Win11 snap-layout flyouts on hover of the maximize button and lets the OS
            // dispatch SC_MINIMIZE/SC_MAXIMIZE/SC_CLOSE.
            if (mTitlebar) {
                if (auto button = mTitlebar->windowButtonAt(local)) {
                    switch (*button) {
                        case AWindowControls::Button::MINIMIZE: return HTMINBUTTON;
                        case AWindowControls::Button::MAXIMIZE: return HTMAXBUTTON;
                        case AWindowControls::Button::CLOSE: return HTCLOSE;
                    }
                }
            }

            if (isResizable() && !IsZoomed(hwnd)) {
                const int border = int((6_dp).getValuePx());
                const int corner = int((12_dp).getValuePx());
                const auto size = getSize();
                const bool left = local.x < border, right = local.x >= size.x - border;
                const bool top = local.y < border, bottom = local.y >= size.y - border;
                const bool nearLeft = local.x < corner, nearRight = local.x >= size.x - corner;
                const bool nearTop = local.y < corner, nearBottom = local.y >= size.y - corner;
                if ((top && nearLeft) || (left && nearTop)) return HTTOPLEFT;
                if ((top && nearRight) || (right && nearTop)) return HTTOPRIGHT;
                if ((bottom && nearLeft) || (left && nearBottom)) return HTBOTTOMLEFT;
                if ((bottom && nearRight) || (right && nearBottom)) return HTBOTTOMRIGHT;
                if (top) return HTTOP;
                if (bottom) return HTBOTTOM;
                if (left) return HTLEFT;
                if (right) return HTRIGHT;
            }

            if (isCaptionAt(local)) {
                return HTCAPTION;
            }
            return HTCLIENT;
        }

        case WM_NCMOUSEMOVE: {
            if (mTitlebar) {
                AOptional<AWindowControls::Button> button;
                switch (wParam) {
                    case HTMINBUTTON: button = AWindowControls::Button::MINIMIZE; break;
                    case HTMAXBUTTON: button = AWindowControls::Button::MAXIMIZE; break;
                    case HTCLOSE: button = AWindowControls::Button::CLOSE; break;
                }
                mTitlebar->setWindowButtonNativeHover(button);
                // register for WM_NCMOUSELEAVE so we can clear hover when the cursor exits the non-client area.
                TRACKMOUSEEVENT tme {};
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = hwnd;
                tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
                TrackMouseEvent(&tme);
            }
            break;
        }

        case WM_NCMOUSELEAVE: {
            if (mTitlebar) {
                mTitlebar->setWindowButtonNativeHover(std::nullopt);
            }
            break;
        }

        case WM_NCLBUTTONDOWN: {
            if (hasCustomFrame(*this) && (wParam == HTMINBUTTON || wParam == HTMAXBUTTON || wParam == HTCLOSE)) {
                return 0;
            }
            break;
        }

        case WM_NCLBUTTONUP: {
            if (hasCustomFrame(*this)) {
                switch (wParam) {
                    case HTMINBUTTON:
                        minimize();
                        return 0;
                    case HTMAXBUTTON:
                        if (isMaximized()) {
                            restore();
                        } else {
                            maximize();
                        }
                        return 0;
                    case HTCLOSE:
                        onCloseButtonClicked();
                        return 0;
                }
            }
            break;
        }

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
            mPosition = glm::ivec2(r->left, r->top);
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
                mPosition = getWindowPosition();
                onResize(LOWORD(lParam), HIWORD(lParam));

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

        case WM_SETTINGCHANGE: {
            if (wParam == 0 && lParam != 0 && wcscmp(reinterpret_cast<wchar_t*>(lParam), L"ImmersiveColorSet") == 0) {
                aui::UpdateDarkModeForHWND(hwnd);
            }
            return 0;
        }

        case WM_GETMINMAXINFO: {
            MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);

            const auto minSize = getMinimumSize();
            RECT r = {0, 0, minSize.x, minSize.y};
            if (!hasCustomFrame(*this)) {
                // windows with custom frame do not have non-client area
                AdjustWindowRectEx(&r, GetWindowLongPtr(mHandle, GWL_STYLE), false, GetWindowLongPtr(mHandle, GWL_EXSTYLE));
            }
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


AWindow::~AWindow() {
    destroyNativeWindow();
}

bool AWindow::hasNativeWindowControls() const { return false; }

// DWM does not draw the caption text when the client area is extended to the whole window.
bool AWindow::hasNativeWindowTitle() const { return false; }

bool AWindow::captionPressed(const APointerPressedEvent&) {
    // handled by WM_NCHITTEST
    return false;
}

void AWindow::setTitleImpl(const AString& title) {
    if (!mHandle) {
        return;
    }
    SetWindowTextW(mHandle, aui::win32::toWchar(title).c_str());
}

void AWindow::setResizable(bool resizable) {
    mResizableCache = resizable;
    if (!mHandle || mClosed) {
        return;
    }
    LONG_PTR style = GetWindowLongPtr(mHandle, GWL_STYLE);
    if (resizable) {
        style |= WS_THICKFRAME;
        if (const auto* d = std::get_if<Init::DecorationDefault>(&mInit.decoration); !d || d->buttons.maximize) {
            style |= WS_MAXIMIZEBOX;
        }
    } else {
        style &= ~(LONG_PTR(WS_THICKFRAME) | LONG_PTR(WS_MAXIMIZEBOX));
    }
    SetWindowLongPtr(mHandle, GWL_STYLE, style);
    SetWindowPos(
        mHandle, nullptr, 0, 0, 0, 0,
        SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}

bool AWindow::isResizable() const {
    if (!mHandle) {
        return mResizableCache && !mClosed;
    }
    return (GetWindowLongPtr(mHandle, GWL_STYLE) & WS_THICKFRAME) != 0;
}

float AWindow::fetchDpiFromSystem() const {
    if (mHandle) {
        typedef UINT(WINAPI* GetDpiForWindow_t)(_In_ HWND);
        static auto GetDpiForWindow = (GetDpiForWindow_t) GetProcAddress(GetModuleHandleA("User32.dll"),
                                                                         "GetDpiForWindow");
        if (GetDpiForWindow) {
            return GetDpiForWindow(mHandle) / 96.f;
        } else {
            typedef UINT(WINAPI* GetDpiForSystem_t)();
            static auto GetDpiForSystem = (GetDpiForSystem_t)GetProcAddress(GetModuleHandleA("User32.dll"), "GetDpiForSystem");
            if (GetDpiForSystem) {
                return GetDpiForSystem() / 96.f;
            }
            return 1.f;
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

void AWindow::setGeometry(int x, int y, int width, int height) {
  mPosition = {x, y};
  AViewContainer::setSize({width, height});

  if (!mHandle) return;

  // the actual non-client area: unlike AdjustWindowRectEx, it is zero for custom caption windows, whose
  // WM_NCCALCSIZE makes the client area cover the whole window.
  int nonClientX = 0;
  int nonClientY = 0;
  if (IsIconic(mHandle)) {
      // a minimized window has an empty client rect, so it can't be used to measure the frame.
      if (!hasCustomFrame(*this)) {
          RECT r = {0, 0, 0, 0};
          AdjustWindowRectEx(&r, GetWindowLongPtr(mHandle, GWL_STYLE), false, GetWindowLongPtr(mHandle, GWL_EXSTYLE));
          nonClientX = (r.right - r.left);
          nonClientY = (r.bottom - r.top);
      }
  } else {
      RECT windowRect, clientRect;
      GetWindowRect(mHandle, &windowRect);
      GetClientRect(mHandle, &clientRect);
      nonClientX = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
      nonClientY = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
  }
  MoveWindow(mHandle, x, y, width + nonClientX, height + nonClientY, false);
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

void AWindow::showImpl() {
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

  updateDpi();

  emit shown();
}
void AWindow::activate(const AString&) {
    show();
    if (!mHandle || mClosed) return;
    if (IsIconic(mHandle)) {
        ShowWindow(mHandle, SW_RESTORE);
    }
    // the launching process called AllowSetForegroundWindow(ASFW_ANY) before forwarding activation to us
    SetForegroundWindow(mHandle);
    BringWindowToTop(mHandle);
}

void AWindow::setIcon(const AImage& image) {
  if (!mHandle) return;
  AUI_ASSERT(image.format() & APixelFormat::BYTE);

  mIcon = aui::win32::imageToIcon(image);

  SendMessage(mHandle, WM_SETICON, ICON_BIG, (LPARAM)mIcon.get());
  // the titlebar draws the small icon
  SendMessage(mHandle, WM_SETICON, ICON_SMALL, (LPARAM)mIcon.get());

  mWindowIcon = _new<AImageDrawable>(_new<AImage>(image));
  emit windowIconChanged();
}

void AWindow::hideImpl() {
  if (!mHandle) return;
  ShowWindow(mHandle, SW_HIDE);
}

static std::atomic<DWORD> gLoopThreadId = 0;

void AWindowManager::notifyProcessMessages() {
    if (!mWindows.empty()) {
        auto& lastWindow = mWindows.back();

        // we don't need to notify MS Windows' message queue about new message if message sent from UI thread.
        if (lastWindow->getThread() != AThread::current()) {
            PostMessage(lastWindow->mHandle, WM_USER, 0, 0);
        }
        return;
    }
    // no windows (i.e., app is held by AApplication::hold()); wake up the thread message queue directly.
    if (auto tid = gLoopThreadId.load(); tid != 0 && tid != GetCurrentThreadId()) {
        PostThreadMessage(tid, WM_USER, 0, 0);
    }
}


void AWindowManager::loop() {
    MSG msg;
    // ensure the thread has a message queue before publishing its id
    PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
    gLoopThreadId = GetCurrentThreadId();
    AUI_DEFER { gLoopThreadId = 0; };
    for (mLoopRunning = true; shouldKeepRunning();) {
        // the queue might have been filled before gLoopThreadId was published
        AThread::processMessages();
        if (!shouldKeepRunning()) {
            break;
        }
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
    if (!mHandle) {
        return;
    }
    EnableWindow(mHandle, !blockUserInput);
    if (!blockUserInput) {
        // the parent of a modal window has to be activated BEFORE the child is closed, otherwise Windows activates
        // some other application.
        SetActiveWindow(mHandle);
        SetForegroundWindow(mHandle);
    }
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
