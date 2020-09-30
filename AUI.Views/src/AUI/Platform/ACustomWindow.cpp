#include "ACustomWindow.h"

#if defined(_WIN32)
#include <glm/gtc/matrix_transform.hpp>

#include "AUI/Render/Render.h"
#include "AUI/Common/AColor.h"
#include "AUI/Image/Drawables.h"
#include "AUI/Render/AFontManager.h"

#include <dwmapi.h>


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
    case WM_NCHITTEST:
    {
        unsigned result = 0;
        const LONG border_width = 8; //in pixels
        RECT winrect;
        GetWindowRect(getNativeHandle(), &winrect);

        auto x = GET_X_LPARAM(lParam);
        auto y = GET_Y_LPARAM(lParam);

        bool resizeWidth = true;//window->minimumWidth() != window->maximumWidth();
        bool resizeHeight = true;//window->minimumHeight() != window->maximumHeight();

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
        if (!result && y - winrect.top <= 30) {
            if (auto v = getViewAtRecusrive({ x - winrect.left, y - winrect.top })) {
                if (!v->getCssNames().contains("AButton") && !v->getCssNames().contains(".override_title_dragging")) {
                    result = HTCAPTION;
                }
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

    RECT rcClient;
    GetWindowRect(getNativeHandle(), &rcClient);

    // Inform the application of the frame change.
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
#else

ACustomWindow::ACustomWindow(const AString& name, int width, int height) :
        AWindow(name, width, height) {

    setWindowStyle(WS_NO_DECORATORS);
}

#endif