/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "AUI/Platform/ACustomWindow.h"
#include "AUI/Platform/ADesktop.h"
#include <cstring>
#include <AUI/View/AButton.h>

const int AUI_TITLE_HEIGHT = 30;

#if AUI_PLATFORM_WIN
#include <glm/gtc/matrix_transform.hpp>

#include "AUI/Render/Render.h"
#include "AUI/Common/AColor.h"
#include "AUI/Platform/AFontManager.h"

#include <dwmapi.h>
#include <AUI/Util/kAUI.h>


LRESULT ACustomWindow::winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
            emit dragBegin();
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

ACustomWindow::ACustomWindow(const AString& name, int width, int height): AWindow(nullptr)
{
    // init here to be sure vtable for wndProc for WM_CREATE is initialized
    windowNativePreInit(name, width, height, nullptr, WindowStyle::DEFAULT);
}

ACustomWindow::ACustomWindow(): ACustomWindow("My custom window", 854, 500)
{
}

ACustomWindow::~ACustomWindow()
{
}

void ACustomWindow::setSize(int width, int height)
{
    AViewContainer::setSize(width, height);
    auto pos = getWindowPosition();

    MoveWindow(mHandle, pos.x, pos.y, width, height, false);
}

#elif AUI_PLATFORM_ANDROID

void ACustomWindow::handleXConfigureNotify() {

}

ACustomWindow::ACustomWindow(const AString &name, int width, int height) {

}

void ACustomWindow::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    ABaseWindow::onMousePressed(pos, button);
}

void ACustomWindow::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AViewContainer::onMouseReleased(pos, button);
}

#elif AUI_PLATFORM_APPLE

ACustomWindow::ACustomWindow(const AString& name, int width, int height) :
        AWindow(name, width, height) {


    setWindowStyle(WindowStyle::NO_DECORATORS);
}
void ACustomWindow::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    if (pos.y < AUI_TITLE_HEIGHT && button == AInput::LButton) {
        if (isCaptionAt(pos)) {
            // TODO apple

            mDragging = true;
            mDragPos = pos;
            emit dragBegin(pos);
        }
    }
    AViewContainer::onMousePressed(pos, button);
}


void ACustomWindow::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AViewContainer::onMouseReleased(pos, button);
}
void ACustomWindow::handleXConfigureNotify() {
    emit dragEnd();

    // x11 does not send release button event
    AViewContainer::onMouseReleased(mDragPos, AInput::LButton);
}


#else

extern Display* gDisplay;

ACustomWindow::ACustomWindow(const AString& name, int width, int height) :
        AWindow(name, width, height) {


    setWindowStyle(WindowStyle::NO_DECORATORS);
}

void ACustomWindow::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    if (pos.y < AUI_TITLE_HEIGHT && button == AInput::LButton) {
        if (isCaptionAt(pos)) {
            XClientMessageEvent xclient;
            memset(&xclient, 0, sizeof(XClientMessageEvent));
            XUngrabPointer(gDisplay, 0);
            XFlush(gDisplay);
            xclient.type = ClientMessage;
            xclient.window = mHandle;
            xclient.message_type = XInternAtom(gDisplay, "_NET_WM_MOVERESIZE", False);
            xclient.format = 32;
            auto newPos = ADesktop::getMousePosition();
            xclient.data.l[0] = newPos.x;
            xclient.data.l[1] = newPos.y;
            xclient.data.l[2] = 8;
            xclient.data.l[3] = 0;
            xclient.data.l[4] = 0;
            XSendEvent(gDisplay, XRootWindow(gDisplay, 0), False, SubstructureRedirectMask | SubstructureNotifyMask,
                       (XEvent*) &xclient);

            mDragging = true;
            mDragPos = pos;
            emit dragBegin(pos);
        }
    }
    AViewContainer::onMousePressed(pos, button);
}


void ACustomWindow::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AViewContainer::onMouseReleased(pos, button);
}
void ACustomWindow::handleXConfigureNotify() {
    emit dragEnd();

    // x11 does not send release button event
    AViewContainer::onMouseReleased(mDragPos, AInput::LButton);
}



#endif


bool ACustomWindow::isCaptionAt(const glm::ivec2& pos) {
    if (pos.y <= AUI_TITLE_HEIGHT) {
        if (auto v = getViewAtRecursive(pos)) {
            if (!(_cast<AButton>(v)) &&
                !v->getCssNames().contains(".override-title-dragging")) {
                return true;
            }
        }
    }
    return false;
}