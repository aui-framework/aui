// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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


#include "AUI/GL/gl.h"
#include "AUI/GL/GLDebug.h"
#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/Render.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Util/ARandom.h"
#include "AUI/GL/State.h"
#include "AUI/Thread/AThread.h"
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Platform/Platform.h"
#include "AUI/Platform/ACustomWindow.h"
#include "AUI/Platform/OpenGLRenderingContext.h"
#include "AUI/UITestState.h"

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>

#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>

#include <X11/extensions/sync.h>



AWindow::~AWindow() {
    mRenderingContext->destroyNativeWindow(*this);
}


void AWindow::quit() {
    getWindowManager().mWindows.removeIf([this](const auto& p) {
        return p.get() == this;
    });

    XUnmapWindow(CommonRenderingContext::ourDisplay, mHandle);

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
}

void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;
    if (!mHandle) return;
    if (!!(ws & (WindowStyle::SYS | WindowStyle::NO_DECORATORS))) {
        // note the struct is declared elsewhere, is here just for clarity.
        // code is from [http://tonyobryan.com/index.php?article=9][1]
        typedef struct Hints
        {
            unsigned long   flags;
            unsigned long   functions;
            unsigned long   decorations;
            long            inputMode;
            unsigned long   status;
        } Hints;

        /*
         * flags:
         * LS
         *  0 - close disable
         *  1 - decorations disable
         * MS
         */
        Hints hints;

        //code to remove decoration
        hints.flags = 2;
        hints.decorations = 0;
        XChangeProperty(CommonRenderingContext::ourDisplay, mHandle, CommonRenderingContext::ourAtoms.wmHints, CommonRenderingContext::ourAtoms.wmHints, 32, PropModeReplace,
                        (unsigned char *)&hints, 5);
    }
}

float AWindow::fetchDpiFromSystem() const {
    return Platform::getDpiRatio();
}

void AWindow::restore() {
    if (CommonRenderingContext::ourAtoms.netWmState &&
        CommonRenderingContext::ourAtoms.netWmStateMaximizedVert &&
        CommonRenderingContext::ourAtoms.netWmStateMaximizedHorz)
        {
            xSendEventToWM(CommonRenderingContext::ourAtoms.netWmState,
                           0,
                           CommonRenderingContext::ourAtoms.netWmStateMaximizedVert,
                           CommonRenderingContext::ourAtoms.netWmStateMaximizedHorz,
                           1, 0);
        }
}

void AWindow::minimize() {
    if (!mHandle) return;
    XIconifyWindow(CommonRenderingContext::ourDisplay, mHandle, 0);
}

bool AWindow::isMinimized() const {
    if (!mHandle) return false;
    int result = WithdrawnState;
    struct {
        uint32_t state;
        Window icon;
    } *state = NULL;

    if (xGetWindowProperty(CommonRenderingContext::ourAtoms.wmState, CommonRenderingContext::ourAtoms.wmState, (unsigned char**) &state) >= 2)
    {
        result = state->state;
    }

    if (state)
        XFree(state);

    return result == IconicState;
}


bool AWindow::isMaximized() const {
    if (!mHandle) return false;
    Atom* states;
    unsigned long i;
    bool maximized = false;

    if (!CommonRenderingContext::ourAtoms.netWmState ||
        !CommonRenderingContext::ourAtoms.netWmStateMaximizedVert ||
        !CommonRenderingContext::ourAtoms.netWmStateMaximizedHorz)
    {
        return maximized;
    }

    const unsigned long count = xGetWindowProperty(CommonRenderingContext::ourAtoms.netWmState, XA_ATOM, (unsigned char**) &states);

    for (i = 0;  i < count;  i++)
    {
        if (states[i] == CommonRenderingContext::ourAtoms.netWmStateMaximizedVert ||
            states[i] == CommonRenderingContext::ourAtoms.netWmStateMaximizedHorz)
        {
            maximized = true;
            break;
        }
    }

    if (states)
        XFree(states);

    return maximized;
}

void AWindow::maximize() {
    if (!mHandle) return;
    // https://github.com/glfw/glfw/blob/master/src/x11_window.c#L2355

    if (!CommonRenderingContext::ourAtoms.netWmState ||
        !CommonRenderingContext::ourAtoms.netWmStateMaximizedVert ||
        !CommonRenderingContext::ourAtoms.netWmStateMaximizedHorz)
    {
        return;
    }

    XWindowAttributes wa;
    XGetWindowAttributes(CommonRenderingContext::ourDisplay, mHandle, &wa);

    if (wa.map_state == IsViewable) {
        xSendEventToWM(CommonRenderingContext::ourAtoms.netWmState, 1, CommonRenderingContext::ourAtoms.netWmStateMaximizedHorz, CommonRenderingContext::ourAtoms.netWmStateMaximizedVert, 0, 0);
    } else {

        Atom* states = NULL;
        unsigned long count =
                xGetWindowProperty(CommonRenderingContext::ourAtoms.netWmState,
                                          XA_ATOM,
                                          (unsigned char**) &states);

        // NOTE: We don't check for failure as this property may not exist yet
        //       and that's fine (and we'll create it implicitly with append)

        Atom missing[2] =
                {
                        CommonRenderingContext::ourAtoms.netWmStateMaximizedVert,
                        CommonRenderingContext::ourAtoms.netWmStateMaximizedHorz
                };
        unsigned long missingCount = 2;

        for (unsigned long i = 0;  i < count;  i++)
        {
            for (unsigned long j = 0;  j < missingCount;  j++)
            {
                if (states[i] == missing[j])
                {
                    missing[j] = missing[missingCount - 1];
                    missingCount--;
                }
            }
        }

        if (states)
            XFree(states);

        if (!missingCount)
            return;

        XChangeProperty(CommonRenderingContext::ourDisplay, mHandle,
                        CommonRenderingContext::ourAtoms.netWmState, XA_ATOM, 32,
                        PropModeAppend,
                        (unsigned char*) missing,
                        missingCount);
    }
    XFlush(CommonRenderingContext::ourDisplay);
}

glm::ivec2 AWindow::getWindowPosition() const {
    if (!mHandle) return {0, 0};
    int x, y;
    Window child;
    XWindowAttributes xwa;
    XTranslateCoordinates(CommonRenderingContext::ourDisplay,
                          mHandle,
                          CommonRenderingContext::ourScreen->root,
                          0, 0,
                          &x, &y,
                          &child);
    XGetWindowAttributes(CommonRenderingContext::ourDisplay, mHandle, &xwa);

    return {x, y};
}


void AWindow::flagRedraw() {
    mRedrawFlag = true;
}
void AWindow::show() {
    if (!getWindowManager().mWindows.contains(_cast<AWindow>(sharedPtr()))) {
        getWindowManager().mWindows << _cast<AWindow>(sharedPtr());
    }
    try {
        mSelfHolder = _cast<AWindow>(sharedPtr());
    } catch (...) {
        mSelfHolder = nullptr;
    }
    AThread::current() << [&]() {
        redraw();
    };
    if (bool(CommonRenderingContext::ourDisplay) && mHandle) {
        XMapWindow(CommonRenderingContext::ourDisplay, mHandle);
    }

    emit shown();
}

void AWindow::setSize(glm::ivec2 size) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, size.x, size.y);

    if (!mHandle) return;
    if (!!(mWindowStyle & WindowStyle::NO_RESIZE)) {
        // we should set min size and max size the same as current size
        XSizeHints* sizehints = XAllocSizeHints();
        long userhints;

        XGetWMNormalHints(CommonRenderingContext::ourDisplay, mHandle, sizehints, &userhints);

        sizehints->min_width = sizehints->min_width = sizehints->max_width = sizehints->base_width = size.x;
        sizehints->min_height = sizehints->min_height = sizehints->max_height = sizehints->base_height = size.y;
        sizehints->flags |= PMinSize | PMaxSize;

        XSetWMNormalHints(CommonRenderingContext::ourDisplay, mHandle, sizehints);

        XFree(sizehints);
    } else {
        XSizeHints* sizehints = XAllocSizeHints();
        long userhints;

        XGetWMNormalHints(CommonRenderingContext::ourDisplay, mHandle, sizehints, &userhints);

        sizehints->min_width = getMinimumWidth();
        sizehints->min_height = getMinimumHeight();
        sizehints->flags |= PMinSize;

        XSetWMNormalHints(CommonRenderingContext::ourDisplay, mHandle, sizehints);

        XFree(sizehints);
    }
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize({width, height});

    if (!mHandle) return;
    XMoveWindow(CommonRenderingContext::ourDisplay, mHandle, x, y);
    XResizeWindow(CommonRenderingContext::ourDisplay, mHandle, width, height);
}

glm::ivec2 AWindow::mapPosition(const glm::ivec2& position) {
    return position - getWindowPosition();
}
glm::ivec2 AWindow::unmapPosition(const glm::ivec2& position) {
    return position + getWindowPosition();
}


void AWindow::setIcon(const AImage& image) {
}

void AWindow::hide() {
    if (!mHandle) return;
    XUnmapWindow(CommonRenderingContext::ourDisplay, mHandle);
}

// HELPER FUNCTIONS FOR XLIB

unsigned long AWindow::xGetWindowProperty(Atom property, Atom type, unsigned char** value) const {
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty(CommonRenderingContext::ourDisplay, mHandle, property, 0, std::numeric_limits<long>::max(), false, type, &actualType,
                       &actualFormat, &itemCount, &bytesAfter, value);

    return itemCount;
}

void AWindow::xSendEventToWM(Atom atom, long a, long b, long c, long d, long e) const {
    if (!mHandle) return;
    XEvent event = { 0 };
    event.type = ClientMessage;
    event.xclient.window = mHandle;
    event.xclient.format = 32; // Data is 32-bit longs
    event.xclient.message_type = atom;
    event.xclient.data.l[0] = a;
    event.xclient.data.l[1] = b;
    event.xclient.data.l[2] = c;
    event.xclient.data.l[3] = d;
    event.xclient.data.l[4] = e;

    XSendEvent(CommonRenderingContext::ourDisplay, DefaultRootWindow(CommonRenderingContext::ourDisplay),
               False,
               SubstructureNotifyMask | SubstructureRedirectMask,
               &event);
}


void AWindowManager::notifyProcessMessages() {
    if (!mWindows.empty()) {
        mXNotifyCV.notify_all();
    }
}

void AWindowManager::loop() {
    XEvent ev;
    for (mLoopRunning = true; mLoopRunning && !mWindows.empty();) {
        xProcessEvent(ev);
    }
}

void AWindowManager::xProcessEvent(XEvent& ev) {
    struct NotFound {};
    auto locateWindow = [&](Window xWindow) -> _<AWindow> {
        for (auto& w : mWindows) {
            if (w->mHandle == xWindow) {
                AWindow::currentWindowStorage() = w.get();
                return w;
            }
        }
        throw NotFound();
    };
    try {
        while (XPending(CommonRenderingContext::ourDisplay)) {
            XNextEvent(CommonRenderingContext::ourDisplay, &ev);
            _<AWindow> window;
            switch (ev.type) {
                case ClientMessage: {
                    if (ev.xclient.message_type == CommonRenderingContext::ourAtoms.wmProtocols) {
                        auto window = locateWindow(ev.xclient.window);
                        if(ev.xclient.data.l[0] == CommonRenderingContext::ourAtoms.wmDeleteWindow) {
                            // close button clicked
                            window->onCloseButtonClicked();
                        } else if (ev.xclient.data.l[0] == CommonRenderingContext::ourAtoms.netWmSyncRequest) {
                            // flicker-fix sync on resize
                            window->mXsyncRequestCounter.lo = ev.xclient.data.l[2];
                            window->mXsyncRequestCounter.hi = ev.xclient.data.l[3];
                        }
                    }
                    break;
                }
                case KeyPress: {
                    window = locateWindow(ev.xkey.window);
                    int count = 0;
                    KeySym keysym = 0;
                    char buf[0x20];
                    Status status = 0;
                    count = Xutf8LookupString((XIC)window->mIC, (XKeyPressedEvent*) &ev, buf, sizeof(buf), &keysym,
                                              &status);

                    // delete key
                    if (buf[0] != 127) {
                        if (count) {
                            AString s(buf);
                            assert(!s.empty());
                            window->onCharEntered(s[0]);
                        }
                    }
                    window->onKeyDown(AInput::fromNative(ev.xkey.keycode));
                    break;
                }
                case KeyRelease:
                    if (XEventsQueued(CommonRenderingContext::ourDisplay, QueuedAfterReading)) // check for key repeat
                    {
                        XEvent nextEvent;
                        XPeekEvent(CommonRenderingContext::ourDisplay, &nextEvent);

                        if (nextEvent.type == KeyPress &&
                            nextEvent.xkey.time == ev.xkey.time &&
                            nextEvent.xkey.keycode == ev.xkey.keycode) {
                            // key wasn't actually released

                            XNextEvent(CommonRenderingContext::ourDisplay, &nextEvent); // consume the event from queue

                            break;
                        }
                    }

                    window = locateWindow(ev.xkey.window);
                    window->onKeyUp(AInput::fromNative(ev.xkey.keycode));
                    break;

                case ConfigureNotify: {
                    window = locateWindow(ev.xconfigure.window);
                    glm::ivec2 size = {ev.xconfigure.width, ev.xconfigure.height};
                    if (size.x >= 10 && size.y >= 10 && size != window->getSize()) {
                        AUI_EMIT_FOREIGN_SIGNAL(window)->resized(size.x, size.y);
                        window->AViewContainer::setSize(size);
                    }
                    if (auto w = _cast<ACustomWindow>(window)) {
                        w->handleXConfigureNotify();
                    }
                    window->mRedrawFlag = false;
                    window->redraw();

                    XSyncValue syncValue;
                    XSyncIntsToValue(&syncValue,
                                     window->mXsyncRequestCounter.lo,
                                     window->mXsyncRequestCounter.hi);
                    XSyncSetCounter(CommonRenderingContext::ourDisplay, window->mXsyncRequestCounter.counter, syncValue);

                    break;
                }

                case MappingNotify:
                    XRefreshKeyboardMapping(&ev.xmapping);
                    break;

                case MotionNotify: {
                    window = locateWindow(ev.xmotion.window);
                    window->onMouseMove({ev.xmotion.x, ev.xmotion.y});
                    break;
                }
                case ButtonPress: {
                    window = locateWindow(ev.xbutton.window);
                    switch (ev.xbutton.button) {
                        case 1:
                        case 2:
                        case 3:
                            window->onMousePressed({ev.xbutton.x, ev.xbutton.y},
                                                   (AInput::Key) (AInput::LBUTTON + ev.xbutton.button - 1));
                            break;
                        case 4: // wheel down
                            window->onMouseWheel({ev.xbutton.x, ev.xbutton.y}, { 0, -120 });
                            break;
                        case 5: // wheel up
                            window->onMouseWheel({ev.xbutton.x, ev.xbutton.y}, { 0, 120 });
                            break;
                    }
                    break;
                }
                case ButtonRelease: {
                    if (ev.xbutton.button < 4) {
                        window = locateWindow(ev.xbutton.window);
                        window->onMouseReleased({ev.xbutton.x, ev.xbutton.y},
                                                (AInput::Key) (AInput::LBUTTON + ev.xbutton.button - 1));
                    }
                    break;
                }

                case PropertyNotify: {
                    window = locateWindow(ev.xproperty.window);
                    if (ev.xproperty.atom == CommonRenderingContext::ourAtoms.netWmState) {
                        auto maximized = window->isMaximized();
                        if (maximized != window->mWasMaximized) {
                            AUI_PERFORM_AS_MEMBER(window, {
                                if (mWasMaximized) {
                                    emit restored();
                                } else {
                                    emit maximized();
                                }
                            });
                            window->mWasMaximized = maximized;
                        }
                    }
                    break;
                }

                case SelectionClear: {
                    // lost clipboard ownership -> clean up
                    mXClipboardText.clear();
                    break;
                }

                case SelectionRequest: {
                    if (ev.xselectionrequest.property == None) {
                        break;
                    }


                    char* targetName = XGetAtomName(CommonRenderingContext::ourDisplay, ev.xselectionrequest.target);
                    char* propertyName = XGetAtomName(CommonRenderingContext::ourDisplay, ev.xselectionrequest.property);
                    ALogger::info("{}: {}"_format(targetName, propertyName));
                    XFree(targetName);
                    XFree(propertyName);
                    if (ev.xselectionrequest.target == CommonRenderingContext::ourAtoms.utf8String ||
                        ev.xselectionrequest.target == CommonRenderingContext::ourAtoms.textPlain ||
                        ev.xselectionrequest.target == CommonRenderingContext::ourAtoms.textPlainUtf8) { // check for UTF8_STRING
                        XChangeProperty(CommonRenderingContext::ourDisplay,
                                        ev.xselectionrequest.requestor,
                                        ev.xselectionrequest.property,
                                        ev.xselectionrequest.target,
                                        8,
                                        PropModeReplace,
                                        (unsigned char*) mXClipboardText.c_str(),
                                        mXClipboardText.length());
                    } else if (ev.xselectionrequest.target == CommonRenderingContext::ourAtoms.targets) { // data type request
                        Atom atoms[] = {
                                XInternAtom(CommonRenderingContext::ourDisplay, "TIMESTAMP", false),
                                XInternAtom(CommonRenderingContext::ourDisplay, "TARGETS", false),
                                XInternAtom(CommonRenderingContext::ourDisplay, "SAVE_TARGETS", false),
                                XInternAtom(CommonRenderingContext::ourDisplay, "MULTIPLE", false),
                                XInternAtom(CommonRenderingContext::ourDisplay, "STRING", false),
                                XInternAtom(CommonRenderingContext::ourDisplay, "UTF8_STRING", false),
                                XInternAtom(CommonRenderingContext::ourDisplay, "text/plain", false),
                                XInternAtom(CommonRenderingContext::ourDisplay, "text/plain;charset=utf-8", false),
                        };
                        XChangeProperty(CommonRenderingContext::ourDisplay,
                                        ev.xselectionrequest.requestor,
                                        ev.xselectionrequest.property,
                                        ev.xselectionrequest.target,
                                        8,
                                        PropModeReplace,
                                        (unsigned char*) atoms,
                                        sizeof(atoms));
                    }

                    XSelectionEvent ssev;
                    ssev.type = SelectionNotify;
                    ssev.requestor = ev.xselectionrequest.requestor;
                    ssev.selection = ev.xselectionrequest.selection;
                    ssev.target = ev.xselectionrequest.target;
                    ssev.property = ev.xselectionrequest.property;
                    ssev.time = ev.xselectionrequest.time;

                    XSendEvent(CommonRenderingContext::ourDisplay, ev.xselectionrequest.requestor, True, NoEventMask, (XEvent *)&ssev);
                    break;
                }
            }
        }

        {
            std::unique_lock lock(mXNotifyLock);
            mXNotifyCV.wait_for(lock, std::chrono::microseconds(500));
        }
        AThread::processMessages();
        if (AWindow::isRedrawWillBeEfficient()) {
            for (auto &window : mWindows) {
                if (window->mRedrawFlag) {
                    window->mRedrawFlag = false;
                    AWindow::currentWindowStorage() = window.get();
                    window->redraw();
                }
            }
        }
    } catch(NotFound e) {

    }
}

AString AWindowManager::xClipboardPasteImpl() {

    auto owner = XGetSelectionOwner(CommonRenderingContext::ourDisplay, CommonRenderingContext::ourAtoms.clipboard);
    if (owner == None)
    {
        return {};
    }
    auto basicWindow = AWindow::current();
    auto auiWindow = dynamic_cast<AWindow*>(basicWindow);
    if (!auiWindow)
        return {};
    auto nativeHandle = auiWindow->getNativeHandle();
    assert(nativeHandle);

    XConvertSelection(CommonRenderingContext::ourDisplay, CommonRenderingContext::ourAtoms.clipboard, CommonRenderingContext::ourAtoms.utf8String, CommonRenderingContext::ourAtoms.auiClipboard, nativeHandle,
                      CurrentTime);

    XEvent ev;
    for (;;)
    {
        XNextEvent(CommonRenderingContext::ourDisplay, &ev);
        switch (ev.type)
        {
            case SelectionNotify: {
                if (ev.xselection.property == None) {
                    return {};
                }
                Atom da, incr, type;
                int di;
                unsigned long size, dul;
                unsigned char *prop_ret = NULL;

                XGetWindowProperty(CommonRenderingContext::ourDisplay, nativeHandle, CommonRenderingContext::ourAtoms.auiClipboard, 0, 0, False, AnyPropertyType,
                                   &type, &di, &dul, &size, &prop_ret);
                XFree(prop_ret);

                if (type == CommonRenderingContext::ourAtoms.incr)
                {
                    ALogger::warn("Clipboard data is too large and INCR mechanism not implemented");
                    return {};
                }

                XGetWindowProperty(CommonRenderingContext::ourDisplay, nativeHandle, CommonRenderingContext::ourAtoms.auiClipboard, 0, size, False, AnyPropertyType,
                                   &da, &di, &dul, &dul, &prop_ret);
                AString clipboardData = (const char*)prop_ret;
                XFree(prop_ret);

                XDeleteProperty(CommonRenderingContext::ourDisplay, nativeHandle, CommonRenderingContext::ourAtoms.auiClipboard);
                return clipboardData;
            }
            default:
                auiWindow->getThread() << [this, ev] {
                    xProcessEvent(const_cast<XEvent&>(ev));
                };
        }
    }
}

void AWindowManager::xClipboardCopyImpl(const AString& text) {
    auto basicWindow = AWindow::current();
    auto auiWindow = dynamic_cast<AWindow*>(basicWindow);
    if (!auiWindow) return;
    mXClipboardText = text.toStdString();
    XSetSelectionOwner(CommonRenderingContext::ourDisplay, CommonRenderingContext::ourAtoms.clipboard, auiWindow->mHandle, CurrentTime);
}

void AWindow::blockUserInput(bool blockUserInput) {
    ABaseWindow::blockUserInput(blockUserInput);
    // TODO linux impl
}

void AWindow::allowDragNDrop() {

}