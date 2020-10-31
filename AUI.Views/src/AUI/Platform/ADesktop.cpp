#include "ADesktop.h"
#include "ACursor.h"
#include "AWindow.h"

#if defined(_WIN32)

#include <Windows.h>
#include <Shlobj.h>
#include <AUI/Traits/memory.h>
#include <AUI/Util/kAUI.h>

glm::ivec2 ADesktop::getMousePosition() {
    POINT p;
    GetCursorPos(&p);
    return {p.x, p.y};
}

void ADesktop::setMousePos(const glm::ivec2& pos) {
    SetCursorPos(pos.x, pos.y);
}

_<AFuture<APath>> ADesktop::browseForFolder() {
    return async {
        APath result;
        BROWSEINFO info;
        aui::zero(info);
        info.ulFlags = BIF_USENEWUI;
        ::OleInitialize(NULL);

        if (AWindow::current()) {
            info.hwndOwner = AWindow::current()->getNativeHandle();
        }
        auto ret = SHBrowseForFolder(&info);
        if (ret) {
            result.resize(4096);
            SHGetPathFromIDList(ret, result.data());
            CoTaskMemFree(ret);
            result.resizeToNullTerminator();
        }
        OleUninitialize();
        return result;
    };
}

#elif defined(ANDROID)
glm::ivec2 ADesktop::getMousePosition()
{
    glm::ivec2 p;
    return p;
}

void ADesktop::setMousePos(const glm::ivec2& pos)
{
}
#else

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
extern Display* gDisplay;
glm::ivec2 ADesktop::getMousePosition()
{
    glm::ivec2 p;
    Window w;
    int unused1;
    unsigned unused2;
    XQueryPointer(gDisplay, XRootWindow(gDisplay, 0), &w, &w, &p.x, &p.y, &unused1, &unused1, &unused2);
    return p;
}

void ADesktop::setMousePos(const glm::ivec2& pos)
{
    auto rootWindow = XRootWindow(gDisplay, 0);
    XSelectInput(gDisplay, rootWindow, KeyReleaseMask);
    XWarpPointer(gDisplay, None, rootWindow, 0, 0, 0, 0, pos.x, pos.y);
    XFlush(gDisplay);
}
#endif