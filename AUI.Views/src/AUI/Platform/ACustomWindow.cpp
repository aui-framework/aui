#include "ACustomWindow.h"
#include "ADesktop.h"
#include <cstring>

const int AUI_TITLE_HEIGHT = 30;

#if defined(_WIN32)
#include <glm/gtc/matrix_transform.hpp>

#include "AUI/Render/Render.h"
#include "AUI/Common/AColor.h"
#include "AUI/Image/Drawables.h"
#include "AUI/Render/AFontManager.h"

#include <dwmapi.h>
#include <AUI/Util/kAUI.h>


LRESULT ACustomWindow::winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
    switch (uMsg)
    {

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

        bool resizeWidth = !(mWindowStyle & WS_NO_RESIZE);//window->minimumWidth() != window->maximumWidth();
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

ACustomWindow::ACustomWindow(const AString& name, int width, int height): AWindow(name, width, height)
{
    AVIEW_CSS;

    const MARGINS shadow = { 1, 1, 1, 1 };
    DwmExtendFrameIntoClientArea((HWND)getNativeHandle(), &shadow);

    // обновить размер окна
    RECT rcClient;
    GetWindowRect(getNativeHandle(), &rcClient);

    SetWindowPos(getNativeHandle(),
                 NULL,
                 rcClient.left, rcClient.top,
                 rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                 SWP_FRAMECHANGED);
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

bool ACustomWindow::isCaptionAt(const glm::ivec2& pos) {
    if (pos.y <= AUI_TITLE_HEIGHT) {
        if (auto v = getViewAtRecursive(pos)) {
            if (!v->getCssNames().contains("AButton") &&
                !v->getCssNames().contains(".override-title-dragging")) {
                return true;
            }
        }
    }
    return false;
}

#else

extern Display* gDisplay;

ACustomWindow::ACustomWindow(const AString& name, int width, int height) :
        AWindow(name, width, height) {
    AVIEW_CSS;

    setWindowStyle(WS_NO_DECORATORS);
}

void ACustomWindow::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    if (pos.y < AUI_TITLE_HEIGHT) {
        auto v = getViewAtRecursive(pos);
        if (!v || !v->getCssNames().contains("AButton")) {
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
}



#endif
