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

#include <X11/Xlib.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

#include "AUI/GL/gl.h"
#include "AUI/GL/GLDebug.h"
#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/CommonRenderingContext.h"
#include "AUI/Platform/ErrorToException.h"
#include "AUI/Render/IRenderer.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Util/ARandom.h"
#include "AUI/GL/State.h"
#include "AUI/Thread/AThread.h"
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Platform/APlatform.h"
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

    if (PlatformAbstractionX11::ourDisplay) {
        XUnmapWindow(PlatformAbstractionX11::ourDisplay, mHandle);
    }

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
        XChangeProperty(PlatformAbstractionX11::ourDisplay, mHandle, PlatformAbstractionX11::ourAtoms.wmHints, PlatformAbstractionX11::ourAtoms.wmHints, 32, PropModeReplace,
                        (unsigned char *)&hints, 5);
    }
}

float AWindow::fetchDpiFromSystem() const {
    return APlatform::getDpiRatio();
}

void AWindow::restore() {
    if (PlatformAbstractionX11::ourAtoms.netWmState &&
        PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert &&
        PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz)
        {
            xSendEventToWM(PlatformAbstractionX11::ourAtoms.netWmState,
                           0,
                           PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert,
                           PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz,
                           1, 0);
        }
}

void AWindow::minimize() {
    if (!mHandle) return;
    XIconifyWindow(PlatformAbstractionX11::ourDisplay, mHandle, 0);
}

bool AWindow::isMinimized() const {
    if (!mHandle) return false;
    int result = WithdrawnState;
    struct {
        uint32_t state;
        Window icon;
    } *state = NULL;

    if (xGetWindowProperty(PlatformAbstractionX11::ourAtoms.wmState, PlatformAbstractionX11::ourAtoms.wmState, (unsigned char**) &state) >= 2)
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

    if (!PlatformAbstractionX11::ourAtoms.netWmState ||
        !PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert ||
        !PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz)
    {
        return maximized;
    }

    const unsigned long count = xGetWindowProperty(PlatformAbstractionX11::ourAtoms.netWmState, XA_ATOM, (unsigned char**) &states);

    for (i = 0;  i < count;  i++)
    {
        if (states[i] == PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert ||
            states[i] == PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz)
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

    if (!PlatformAbstractionX11::ourAtoms.netWmState ||
        !PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert ||
        !PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz)
    {
        return;
    }

    XWindowAttributes wa;
    XGetWindowAttributes(PlatformAbstractionX11::ourDisplay, mHandle, &wa);

    if (wa.map_state == IsViewable) {
        xSendEventToWM(PlatformAbstractionX11::ourAtoms.netWmState, 1, PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz, PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert, 0, 0);
    } else {

        Atom* states = NULL;
        unsigned long count =
                xGetWindowProperty(PlatformAbstractionX11::ourAtoms.netWmState,
                                          XA_ATOM,
                                          (unsigned char**) &states);

        // NOTE: We don't check for failure as this property may not exist yet
        //       and that's fine (and we'll create it implicitly with append)

        Atom missing[2] =
                {
                        PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert,
                        PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz
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

        XChangeProperty(PlatformAbstractionX11::ourDisplay, mHandle,
                        PlatformAbstractionX11::ourAtoms.netWmState, XA_ATOM, 32,
                        PropModeAppend,
                        (unsigned char*) missing,
                        missingCount);
    }
    XFlush(PlatformAbstractionX11::ourDisplay);
}

glm::ivec2 AWindow::getWindowPosition() const {
    if (!mHandle) return {0, 0};
    int x, y;
    Window child;
    XWindowAttributes xwa;
    XTranslateCoordinates(PlatformAbstractionX11::ourDisplay,
                          mHandle,
                          PlatformAbstractionX11::ourScreen->root,
                          0, 0,
                          &x, &y,
                          &child);
    XGetWindowAttributes(PlatformAbstractionX11::ourDisplay, mHandle, &xwa);

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
    if (bool(PlatformAbstractionX11::ourDisplay) && mHandle) {
        AThread::current() << [&]() {
            XMapWindow(PlatformAbstractionX11::ourDisplay, mHandle);
        };
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

        XGetWMNormalHints(PlatformAbstractionX11::ourDisplay, mHandle, sizehints, &userhints);

        sizehints->min_width = sizehints->min_width = sizehints->max_width = sizehints->base_width = size.x;
        sizehints->min_height = sizehints->min_height = sizehints->max_height = sizehints->base_height = size.y;
        sizehints->flags |= PMinSize | PMaxSize;

        XSetWMNormalHints(PlatformAbstractionX11::ourDisplay, mHandle, sizehints);

        XFree(sizehints);
    } else {
        XSizeHints* sizehints = XAllocSizeHints();
        long userhints;

        XGetWMNormalHints(PlatformAbstractionX11::ourDisplay, mHandle, sizehints, &userhints);

        sizehints->min_width = getMinimumWidth();
        sizehints->min_height = getMinimumHeight();
        sizehints->flags |= PMinSize;

        XSetWMNormalHints(PlatformAbstractionX11::ourDisplay, mHandle, sizehints);

        XFree(sizehints);
    }
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize({width, height});

    if (!mHandle) return;
    XMoveWindow(PlatformAbstractionX11::ourDisplay, mHandle, x, y);
    XResizeWindow(PlatformAbstractionX11::ourDisplay, mHandle, width, height);
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
    XUnmapWindow(PlatformAbstractionX11::ourDisplay, mHandle);
}

// HELPER FUNCTIONS FOR XLIB

unsigned long AWindow::xGetWindowProperty(Atom property, Atom type, unsigned char** value) const {
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty(PlatformAbstractionX11::ourDisplay, mHandle, property, 0, std::numeric_limits<long>::max(), false, type, &actualType,
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

    XSendEvent(PlatformAbstractionX11::ourDisplay, DefaultRootWindow(PlatformAbstractionX11::ourDisplay),
               False,
               SubstructureNotifyMask | SubstructureRedirectMask,
               &event);
}


void AWindowManager::notifyProcessMessages() {
    if (mWindows.empty()) {
        return;
    }
    if (mFastPathNotify.exchange(true)) {
        return;
    }
    char dummy = 0;
    int unused = write(mNotifyPipe.in(), &dummy, sizeof(dummy));
}

void AWindowManager::loop() {
    // make sure notifyProcessMessages would not block as it syscalls write to the pipe
    fcntl(mNotifyPipe.in(), F_SETFL, O_NONBLOCK);
    pollfd ps[] = {
        {
            .fd = XConnectionNumber(PlatformAbstractionX11::ourDisplay),
            .events = POLLIN,
        },
        {
            .fd = mNotifyPipe.out(),
            .events = POLLIN,
        },
    };

    XEvent ev;
    for (mLoopRunning = true; mLoopRunning && !mWindows.empty();) {
        try {
            xProcessEvent(ev);
        } catch (const AException& e) {
            ALogger::err("AUI") << "Uncaught exception in window proc: " << e;
        }

        AThread::processMessages();
        if (AWindow::isRedrawWillBeEfficient()) {
            for (auto &window : mWindows) {
                if (window->mRedrawFlag) {
                    window->mRedrawFlag = false;
                    AWindow::currentWindowStorage() = window.get();
                    mWatchdog.runOperation([&] {
                        window->redraw();
                    });
                }
            }
        }

        // [1000 ms timeout] sometimes, leaving an always rerendering window (game) work a long time deadlocks the loop
        // in infinite poll.
        const auto timeout = mFastPathNotify || ranges::any_of(mWindows, [](const _<AWindow>& window) { return window->mRedrawFlag; }) ? 0 : 1000;
        if (int p = poll(ps, std::size(ps), timeout); p < 0) {
            aui::impl::unix_based::lastErrorToException("eventloop poll failed");
        } else if (p == 0) {
            continue;
        }
        mFastPathNotify = false;
        if (ps[1].revents & POLLIN) {
            char unused[0xff];
            while (read(mNotifyPipe.out(), unused, std::size(unused)) == std::size(unused));
        }
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
        while (XPending(PlatformAbstractionX11::ourDisplay)) {
            mWatchdog.runOperation([&] {
                XNextEvent(PlatformAbstractionX11::ourDisplay, &ev);
                _<AWindow> window;
                switch (ev.type) {
                    case Expose: {
                        window = locateWindow(ev.xexpose.window);
                        window->flagRedraw();
                        break;
                    }
                    case ClientMessage: {
                        if (ev.xclient.message_type == PlatformAbstractionX11::ourAtoms.wmProtocols) {
                            auto window = locateWindow(ev.xclient.window);
                            if(ev.xclient.data.l[0] == PlatformAbstractionX11::ourAtoms.wmDeleteWindow) {
                                // close button clicked
                                window->onCloseButtonClicked();
                            } else if (ev.xclient.data.l[0] == PlatformAbstractionX11::ourAtoms.netWmSyncRequest) {
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

                        if (count > 0) {
                            switch (buf[0]) {
                                case 27: break; // esc
                                case 127: break; // del
                                default:
                                    AString s(buf);
                                    AUI_ASSERT(!s.empty());
                                    window->onCharEntered(s[0]);
                            }
                        }
                        window->onKeyDown(AInput::fromNative(ev.xkey.keycode));
                        break;
                    }
                    case KeyRelease:
                        if (XEventsQueued(PlatformAbstractionX11::ourDisplay, QueuedAfterReading)) // check for key repeat
                        {
                            XEvent nextEvent;
                            XPeekEvent(PlatformAbstractionX11::ourDisplay, &nextEvent);

                            if (nextEvent.type == KeyPress &&
                                nextEvent.xkey.time == ev.xkey.time &&
                                nextEvent.xkey.keycode == ev.xkey.keycode) {
                                // key wasn't actually released

                                XNextEvent(PlatformAbstractionX11::ourDisplay, &nextEvent); // consume the event from queue

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
                            AUI_NULLSAFE(window->mRenderingContext)->beginResize(*window);
                            window->AViewContainer::setSize(size);
                            AUI_NULLSAFE(window->mRenderingContext)->endResize(*window);
                        }
                        if (auto w = _cast<ACustomWindow>(window)) {
                            w->handleXConfigureNotify();
                        }
                        window->mRedrawFlag = true;

                        XSyncValue syncValue;
                        XSyncIntsToValue(&syncValue,
                                        window->mXsyncRequestCounter.lo,
                                        window->mXsyncRequestCounter.hi);
                        XSyncSetCounter(PlatformAbstractionX11::ourDisplay, window->mXsyncRequestCounter.counter, syncValue);

                        break;
                    }

                    case MappingNotify:
                        XRefreshKeyboardMapping(&ev.xmapping);
                        break;

                    case MotionNotify: {
                        window = locateWindow(ev.xmotion.window);
                        window->onPointerMove({ev.xmotion.x, ev.xmotion.y}, {});
                        AUI_NULLSAFE(window->getCursor())->applyNativeCursor(window.get());
                        break;
                    }
                    case ButtonPress: {
                        window = locateWindow(ev.xbutton.window);

                        const auto SCROLL = 11_pt * 3.f;

                        switch (ev.xbutton.button) {
                            case 1:
                            case 2:
                            case 3:
                                window->onPointerPressed({
                                    .position = { ev.xbutton.x, ev.xbutton.y },
                                    .pointerIndex = APointerIndex::button(
                                            static_cast<AInput::Key>(AInput::LBUTTON + ev.xbutton.button - 1))
                                });
                                break;
                            case 4: // wheel down
                                window->onScroll({                     // TODO libinput
                                    .origin = {ev.xbutton.x, ev.xbutton.y},  //
                                    .delta = { 0, -SCROLL }                     //
                                });                                          //
                                break;                                       //
                            case 5: // wheel up                              //
                                window->onScroll({                     //
                                    .origin = {ev.xbutton.x, ev.xbutton.y},  //
                                    .delta = { 0, SCROLL }                      //
                                });                                          //
                                break;
                        }
                        break;
                    }
                    case ButtonRelease: {
                        if (ev.xbutton.button < 4) {
                            window = locateWindow(ev.xbutton.window);
                            window->onPointerReleased({
                                .position = { ev.xbutton.x, ev.xbutton.y },
                                .pointerIndex = APointerIndex::button(
                                        static_cast<AInput::Key>(AInput::LBUTTON + ev.xbutton.button - 1))
                            });
                        }
                        break;
                    }

                    case PropertyNotify: {
                        window = locateWindow(ev.xproperty.window);
                        if (ev.xproperty.atom == PlatformAbstractionX11::ourAtoms.netWmState) {
                            auto maximized = window->isMaximized();
                            if (maximized != window->mWasMaximized) {
                                if (window->mWasMaximized) {
                                    AUI_EMIT_FOREIGN(window, restored);
                                } else {
                                    AUI_EMIT_FOREIGN(window, maximized);
                                }
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


                        char* targetName = XGetAtomName(PlatformAbstractionX11::ourDisplay, ev.xselectionrequest.target);
                        char* propertyName = XGetAtomName(PlatformAbstractionX11::ourDisplay, ev.xselectionrequest.property);
                        ALogger::info("{}: {}"_format(targetName, propertyName));
                        XFree(targetName);
                        XFree(propertyName);
                        if (ev.xselectionrequest.target == PlatformAbstractionX11::ourAtoms.utf8String ||
                            ev.xselectionrequest.target == PlatformAbstractionX11::ourAtoms.textPlain ||
                            ev.xselectionrequest.target == PlatformAbstractionX11::ourAtoms.textPlainUtf8) { // check for UTF8_STRING
                            XChangeProperty(PlatformAbstractionX11::ourDisplay,
                                            ev.xselectionrequest.requestor,
                                            ev.xselectionrequest.property,
                                            ev.xselectionrequest.target,
                                            8,
                                            PropModeReplace,
                                            (unsigned char*) mXClipboardText.c_str(),
                                            mXClipboardText.length());
                        } else if (ev.xselectionrequest.target == PlatformAbstractionX11::ourAtoms.targets) { // data type request
                            Atom atoms[] = {
                                    XInternAtom(PlatformAbstractionX11::ourDisplay, "TIMESTAMP", false),
                                    XInternAtom(PlatformAbstractionX11::ourDisplay, "TARGETS", false),
                                    XInternAtom(PlatformAbstractionX11::ourDisplay, "SAVE_TARGETS", false),
                                    XInternAtom(PlatformAbstractionX11::ourDisplay, "MULTIPLE", false),
                                    XInternAtom(PlatformAbstractionX11::ourDisplay, "STRING", false),
                                    XInternAtom(PlatformAbstractionX11::ourDisplay, "UTF8_STRING", false),
                                    XInternAtom(PlatformAbstractionX11::ourDisplay, "text/plain", false),
                                    XInternAtom(PlatformAbstractionX11::ourDisplay, "text/plain;charset=utf-8", false),
                            };
                            XChangeProperty(PlatformAbstractionX11::ourDisplay,
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

                        XSendEvent(PlatformAbstractionX11::ourDisplay, ev.xselectionrequest.requestor, True, NoEventMask, (XEvent *)&ssev);
                        break;
                    }
                }
            });
        }

    } catch(NotFound e) {

    }
}

AString AWindowManager::xClipboardPasteImpl() {

    auto owner = XGetSelectionOwner(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::ourAtoms.clipboard);
    if (owner == None)
    {
        return {};
    }
    auto basicWindow = AWindow::current();
    auto auiWindow = dynamic_cast<AWindow*>(basicWindow);
    if (!auiWindow)
        return {};
    auto nativeHandle = auiWindow->getNativeHandle();
    AUI_ASSERT(nativeHandle);

    XConvertSelection(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::ourAtoms.clipboard, PlatformAbstractionX11::ourAtoms.utf8String, PlatformAbstractionX11::ourAtoms.auiClipboard, nativeHandle,
                      CurrentTime);

    XEvent ev;
    for (int i = 0; i < 30; ++i)
    {
        XNextEvent(PlatformAbstractionX11::ourDisplay, &ev);
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

                XGetWindowProperty(PlatformAbstractionX11::ourDisplay, nativeHandle, PlatformAbstractionX11::ourAtoms.auiClipboard, 0, 0, False, AnyPropertyType,
                                   &type, &di, &dul, &size, &prop_ret);
                XFree(prop_ret);

                if (type == PlatformAbstractionX11::ourAtoms.incr)
                {
                    ALogger::warn("Clipboard data is too large and INCR mechanism not implemented");
                    return {};
                }

                XGetWindowProperty(PlatformAbstractionX11::ourDisplay, nativeHandle, PlatformAbstractionX11::ourAtoms.auiClipboard, 0, size, False, AnyPropertyType,
                                   &da, &di, &dul, &dul, &prop_ret);
                AString clipboardData = (const char*)prop_ret;
                XFree(prop_ret);

                XDeleteProperty(PlatformAbstractionX11::ourDisplay, nativeHandle, PlatformAbstractionX11::ourAtoms.auiClipboard);
                return clipboardData;
            }
            default:
                auiWindow->getThread() << [this, ev] {
                    xProcessEvent(const_cast<XEvent&>(ev));
                };
        }
    }
    return "";
}

void AWindowManager::xClipboardCopyImpl(const AString& text) {
    auto basicWindow = AWindow::current();
    auto auiWindow = dynamic_cast<AWindow*>(basicWindow);
    if (!auiWindow) return;
    mXClipboardText = text.toStdString();
    XSetSelectionOwner(PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::ourAtoms.clipboard, auiWindow->mHandle, CurrentTime);
}

void AWindow::blockUserInput(bool blockUserInput) {
    AWindowBase::blockUserInput(blockUserInput);
    // TODO linux impl
}

void AWindow::allowDragNDrop() {

}

void AWindow::showTouchscreenKeyboardImpl() {
    AWindowBase::showTouchscreenKeyboardImpl();
}

void AWindow::hideTouchscreenKeyboardImpl() {
    AWindowBase::hideTouchscreenKeyboardImpl();
}

void AWindow::moveToCenter() {

}

void AWindow::setMobileScreenOrientation(AScreenOrientation screenOrientation) {

}
void AWindow::applyGeometryToChildren() {
    AWindowBase::applyGeometryToChildren();
}
