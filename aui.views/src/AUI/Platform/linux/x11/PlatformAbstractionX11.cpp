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

#include "PlatformAbstractionX11.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/UITestState.h"

aui::assert_not_used_when_null<Display*> PlatformAbstractionX11::ourDisplay = nullptr;
Screen* PlatformAbstractionX11::ourScreen = nullptr;
PlatformAbstractionX11::Atoms PlatformAbstractionX11::ourAtoms;

/**
     * _NET_WM_SYNC_REQUEST (resize flicker fix) update request counter
 */
struct {
    uint32_t lo = 0;
    uint32_t hi = 0;
    /* XID */ unsigned long counter;
} mXsyncRequestCounter;
void* mIC = nullptr; // input context


static int xerrorhandler(Display* dsp, XErrorEvent* error) {
    if (PlatformAbstractionX11::ourDisplay == dsp) {
        char errorstring[0x100];
        XGetErrorText(dsp, error->error_code, errorstring, sizeof(errorstring));
        ALogger::info("X11") << "Error: " << errorstring << "\n" << AStacktrace::capture(2);
    }
    return 0;
}

void PlatformAbstractionX11::ensureXLibInitialized() {
    if (UITestState::isTesting()) {
        return;
    }
    struct DisplayInstance {

    public:
        DisplayInstance() {
            auto d = ourDisplay = XOpenDisplay(nullptr);
            if (d == nullptr) return;
            XSetErrorHandler(xerrorhandler);
            ourScreen = DefaultScreenOfDisplay(ourDisplay);

            ourAtoms.wmProtocols = XInternAtom(d, "WM_PROTOCOLS", False);
            ourAtoms.wmDeleteWindow = XInternAtom(d, "WM_DELETE_WINDOW", False);
            ourAtoms.wmHints = XInternAtom(d, "_MOTIF_WM_HINTS", true);
            ourAtoms.wmState = XInternAtom(d, "WM_STATE", true);
            ourAtoms.netWmState = XInternAtom(d, "_NET_WM_STATE", false);
            ourAtoms.netWmStateMaximizedVert = XInternAtom(d, "_NET_WM_STATE_MAXIMIZED_VERT", false);
            ourAtoms.netWmStateMaximizedHorz = XInternAtom(d, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
            ourAtoms.clipboard = XInternAtom(d, "CLIPBOARD", False);
            ourAtoms.utf8String = XInternAtom(d, "UTF8_STRING", False);
            ourAtoms.textPlain = XInternAtom(d, "text/plain", False);
            ourAtoms.textPlainUtf8 = XInternAtom(d, "text/plain;charset=utf-8", False);
            ourAtoms.auiClipboard = XInternAtom(d, "AUI_CLIPBOARD", False);
            ourAtoms.incr = XInternAtom(d, "INCR", False);
            ourAtoms.targets = XInternAtom(d, "TARGETS", False);
            ourAtoms.netWmSyncRequest = XInternAtom(d, "_NET_WM_SYNC_REQUEST", False);
            ourAtoms.netWmSyncRequestCounter = XInternAtom(d, "_NET_WM_SYNC_REQUEST_COUNTER", False);
        }

        ~DisplayInstance() {
            //XCloseDisplay(ourDisplay);
            //XFree(ourScreen);

        }
    };
    static DisplayInstance display;
}



void PlatformAbstractionX11::xProcessEvent(XEvent& ev) {
    struct NotFound {};
    auto locateWindow = [&](Window xWindow) -> _<AWindow> {
        for (auto& w : AWindow::getWindowManager().getWindows()) {
            if (w->nativeHandle() == xWindow) {
                setCurrentWindow(w.get());
                return w;
            }
        }
        throw NotFound();
    };
    try {
        while (XPending(ourDisplay)) {
            AWindow::getWindowManager().watchdog().runOperation([&] {
                XNextEvent(ourDisplay, &ev);
                _<AWindow> window;
                switch (ev.type) {
                    case Expose: {
                        window = locateWindow(ev.xexpose.window);
                        window->flagRedraw();
                        break;
                    }
                    case ClientMessage: {
                        if (ev.xclient.message_type == ourAtoms.wmProtocols) {
                            auto window = locateWindow(ev.xclient.window);
                            if (ev.xclient.data.l[0] == ourAtoms.wmDeleteWindow) {
                                // close button clicked
                                window->onCloseButtonClicked();
                            } else if (ev.xclient.data.l[0] == ourAtoms.netWmSyncRequest) {
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
                        count = Xutf8LookupString(
                            (XIC) window->mIC, (XKeyPressedEvent*) &ev, buf, sizeof(buf), &keysym, &status);

                        if (count > 0) {
                            switch (buf[0]) {
                                case 27:
                                    break;   // esc
                                case 127:
                                    break;   // del
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
                        if (XEventsQueued(ourDisplay, QueuedAfterReading))   // check for key repeat
                        {
                            XEvent nextEvent;
                            XPeekEvent(ourDisplay, &nextEvent);

                            if (nextEvent.type == KeyPress && nextEvent.xkey.time == ev.xkey.time &&
                                nextEvent.xkey.keycode == ev.xkey.keycode) {
                                // key wasn't actually released

                                XNextEvent(ourDisplay, &nextEvent);   // consume the event from queue

                                break;
                            }
                        }

                        window = locateWindow(ev.xkey.window);
                        window->onKeyUp(AInput::fromNative(ev.xkey.keycode));
                        break;

                    case ConfigureNotify: {
                        window = locateWindow(ev.xconfigure.window);
                        glm::ivec2 size = { ev.xconfigure.width, ev.xconfigure.height };
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
                        XSyncIntsToValue(&syncValue, window->mXsyncRequestCounter.lo, window->mXsyncRequestCounter.hi);
                        XSyncSetCounter(ourDisplay, window->mXsyncRequestCounter.counter, syncValue);

                        break;
                    }

                    case MappingNotify:
                        XRefreshKeyboardMapping(&ev.xmapping);
                        break;

                    case MotionNotify: {
                        window = locateWindow(ev.xmotion.window);
                        window->onPointerMove({ ev.xmotion.x, ev.xmotion.y }, {});
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
                                window->onPointerPressed(
                                    { .position = { ev.xbutton.x, ev.xbutton.y },
                                      .pointerIndex = APointerIndex::button(
                                          static_cast<AInput::Key>(AInput::LBUTTON + ev.xbutton.button - 1)) });
                                break;
                            case 4:   // wheel down
                                window->onScroll({ .origin = { ev.xbutton.x, ev.xbutton.y }, .delta = { 0, -SCROLL } });
                                break;
                            case 5:   // wheel up
                                window->onScroll({ .origin = { ev.xbutton.x, ev.xbutton.y }, .delta = { 0, SCROLL } });
                                break;
                        }
                        break;
                    }
                    case ButtonRelease: {
                        if (ev.xbutton.button < 4) {
                            window = locateWindow(ev.xbutton.window);
                            window->onPointerReleased(
                                { .position = { ev.xbutton.x, ev.xbutton.y },
                                  .pointerIndex = APointerIndex::button(
                                      static_cast<AInput::Key>(AInput::LBUTTON + ev.xbutton.button - 1)) });
                        }
                        break;
                    }

                    case PropertyNotify: {
                        window = locateWindow(ev.xproperty.window);
                        if (ev.xproperty.atom == ourAtoms.netWmState) {
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
                        xClipboardClear();
                        break;
                    }

                    case SelectionRequest: {
                        xHandleClipboard(ev.xselectionrequest);
                        break;
                    }
                }
            });
        }

    } catch (NotFound e) {
    }
}

void PlatformAbstractionX11::windowQuit(AWindow& window) {
    if (ourDisplay) {
        XUnmapWindow(ourDisplay, nativeHandle(window));
    }
}

float PlatformAbstractionX11::windowFetchDpiFromSystem(AWindow& window) {
    return APlatform::getDpiRatio();
}

void PlatformAbstractionX11::windowRestore(AWindow& window) {
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

void PlatformAbstractionX11::windowMinimize(AWindow& window) {
    if (!nativeHandle(window)) return;
    XIconifyWindow(PlatformAbstractionX11::ourDisplay, nativeHandle(window), 0);
}

bool PlatformAbstractionX11::windowIsMinimized(AWindow& window) const {
    if (!nativeHandle(window)) return false;
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


bool PlatformAbstractionX11::windowIsMaximized() const {
    if (!nativeHandle(window)) return false;
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

void PlatformAbstractionX11::windowMaximize(AWindow& window) {
    if (!nativeHandle(window)) return;
    // https://github.com/glfw/glfw/blob/master/src/x11_window.c#L2355

    if (!PlatformAbstractionX11::ourAtoms.netWmState ||
        !PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert ||
        !PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz)
    {
        return;
    }

    XWindowAttributes wa;
    XGetWindowAttributes(PlatformAbstractionX11::ourDisplay, nativeHandle(window), &wa);

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

        XChangeProperty(PlatformAbstractionX11::ourDisplay, nativeHandle(window),
                        PlatformAbstractionX11::ourAtoms.netWmState, XA_ATOM, 32,
                        PropModeAppend,
                        (unsigned char*) missing,
                        missingCount);
    }
    XFlush(PlatformAbstractionX11::ourDisplay);
}

glm::ivec2 PlatformAbstractionX11::windowGetWindowPosition() const {
    if (!nativeHandle(window)) return {0, 0};
    int x, y;
    Window child;
    XWindowAttributes xwa;
    XTranslateCoordinates(PlatformAbstractionX11::ourDisplay,
                          nativeHandle(window),
                          PlatformAbstractionX11::ourScreen->root,
                          0, 0,
                          &x, &y,
                          &child);
    XGetWindowAttributes(PlatformAbstractionX11::ourDisplay, nativeHandle(window), &xwa);

    return {x, y};
}


void PlatformAbstractionX11::windowFlagRedraw(AWindow& window) {
    mRedrawFlag = true;
}
void PlatformAbstractionX11::windowShow(AWindow& window) {
    if (!getWindowManager().mWindows.contains(_cast<AWindow>(sharedPtr()))) {
        getWindowManager().mWindows << _cast<AWindow>(sharedPtr());
    }
    try {
        mSelfHolder = _cast<AWindow>(sharedPtr());
    } catch (...) {
        mSelfHolder = nullptr;
    }
    if (bool(PlatformAbstractionX11::ourDisplay) && nativeHandle(window)) {
        AThread::current() << [&]() {
          XMapWindow(PlatformAbstractionX11::ourDisplay, nativeHandle(window));
        };
    }

    emit shown();
}

void PlatformAbstractionX11::windowSetSize(AWindow& window, glm::ivec2 size) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, size.x, size.y);

    if (!nativeHandle(window)) return;
    if (!!(mWindowStyle & WindowStyle::NO_RESIZE)) {
        // we should set min size and max size the same as current size
        XSizeHints* sizehints = XAllocSizeHints();
        long userhints;

        XGetWMNormalHints(PlatformAbstractionX11::ourDisplay, nativeHandle(window), sizehints, &userhints);

        sizehints->min_width = sizehints->min_width = sizehints->max_width = sizehints->base_width = size.x;
        sizehints->min_height = sizehints->min_height = sizehints->max_height = sizehints->base_height = size.y;
        sizehints->flags |= PMinSize | PMaxSize;

        XSetWMNormalHints(PlatformAbstractionX11::ourDisplay, nativeHandle(window), sizehints);

        XFree(sizehints);
    } else {
        XSizeHints* sizehints = XAllocSizeHints();
        long userhints;

        XGetWMNormalHints(PlatformAbstractionX11::ourDisplay, nativeHandle(window), sizehints, &userhints);

        sizehints->min_width = getMinimumWidth();
        sizehints->min_height = getMinimumHeight();
        sizehints->flags |= PMinSize;

        XSetWMNormalHints(PlatformAbstractionX11::ourDisplay, nativeHandle(window), sizehints);

        XFree(sizehints);
    }
}

void PlatformAbstractionX11::windowSetGeometry(AWindow& window, int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize({width, height});

    if (!nativeHandle(window)) return;
    XMoveWindow(PlatformAbstractionX11::ourDisplay, nativeHandle(window), x, y);
    XResizeWindow(PlatformAbstractionX11::ourDisplay, nativeHandle(window), width, height);
}

glm::ivec2 PlatformAbstractionX11::windowMapPosition(AWindow& window, const glm::ivec2& position) {
    return position - getWindowPosition();
}
glm::ivec2 PlatformAbstractionX11::windowUnmapPosition(AWindow& window, const glm::ivec2& position) {
    return position + getWindowPosition();
}


void PlatformAbstractionX11::windowSetIcon(AWindow& window, const AImage& image) {
}

void PlatformAbstractionX11::windowHide(AWindow& window) {
    if (!nativeHandle(window)) return;
    XUnmapWindow(PlatformAbstractionX11::ourDisplay, nativeHandle(window));
}

// HELPER FUNCTIONS FOR XLIB

unsigned long PlatformAbstractionX11::windowXGetWindowProperty(Atom property, Atom type, unsigned char** value) const {
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty(PlatformAbstractionX11::ourDisplay, nativeHandle(window), property, 0, std::numeric_limits<long>::max(), false, type, &actualType,
                       &actualFormat, &itemCount, &bytesAfter, value);

    return itemCount;
}

void PlatformAbstractionX11::windowXSendEventToWM(Atom atom, long a, long b, long c, long d, long e) const {
    if (!nativeHandle(window)) return;
    XEvent event = { 0 };
    event.type = ClientMessage;
    event.xclient.window = nativeHandle(window);
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


void PlatformAbstractionX11::windowManagerNotifyProcessMessages() {
    if (mWindows.empty()) {
        return;
    }
    if (mFastPathNotify.exchange(true)) {
        return;
    }
    char dummy = 0;
    int unused = write(mNotifyPipe.in(), &dummy, sizeof(dummy));
}

void PlatformAbstractionX11::windowManagerLoop() {
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
        if (PlatformAbstractionX11::windowIsRedrawWillBeEfficient()) {
            for (auto &window : mWindows) {
                if (window->mRedrawFlag) {
                    window->mRedrawFlag = false;
                    PlatformAbstractionX11::windowCurrentWindowStorage() = window.get();
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

void PlatformAbstractionX11::windowBlockUserInput(AWindow& window, bool blockUserInput) {
    AWindowBase::blockUserInput(blockUserInput);
    // TODO linux impl
}

void PlatformAbstractionX11::windowAllowDragNDrop(AWindow& window) {

}

void PlatformAbstractionX11::windowShowTouchscreenKeyboardImpl(AWindow& window) {
    AWindowBase::showTouchscreenKeyboardImpl();
}

void PlatformAbstractionX11::windowHideTouchscreenKeyboardImpl(AWindow& window) {
    AWindowBase::hideTouchscreenKeyboardImpl();
}

void PlatformAbstractionX11::windowMoveToCenter(AWindow& window) {

}
