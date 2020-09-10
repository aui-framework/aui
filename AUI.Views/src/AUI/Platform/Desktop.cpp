#include "Desktop.h"
#include "ACursor.h"

#if defined(_WIN32)
#include <Windows.h>
glm::ivec2 Desktop::getMousePos()
{
	POINT p;
	GetCursorPos(&p);
	return { p.x, p.y };
}

void Desktop::setMousePos(const glm::ivec2& pos)
{
	SetCursorPos(pos.x, pos.y);
}
#else

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
extern Display* gDisplay;
glm::ivec2 Desktop::getMousePos()
{
    glm::ivec2 p;
    XQueryPointer(gDisplay, XRootWindow(gDisplay, 0), nullptr, nullptr, &p.x, &p.y, nullptr, nullptr, nullptr);
    return p;
}

void Desktop::setMousePos(const glm::ivec2& pos)
{
    auto rootWindow = XRootWindow(gDisplay, 0);
    XSelectInput(gDisplay, rootWindow, KeyReleaseMask);
    XWarpPointer(gDisplay, None, rootWindow, 0, 0, 0, 0, pos.x, pos.y);
    XFlush(gDisplay);
}
#endif