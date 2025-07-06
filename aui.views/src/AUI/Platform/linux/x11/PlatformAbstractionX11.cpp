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

#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

#include "PlatformAbstractionX11.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/UITestState.h"
#include "RenderingContextX11.h"
#include "AUI/Platform/ACustomWindow.h"
#include "AUI/Platform/AWindowManager.h"
#include "AUI/Platform/ErrorToException.h"
#include "AUI/Platform/ARenderingContextOptions.h"
#include "OpenGLRenderingContextX11.h"
#include "SoftwareRenderingContextX11.h"

aui::assert_not_used_when_null<Display*> PlatformAbstractionX11::ourDisplay = nullptr;
Screen* PlatformAbstractionX11::ourScreen = nullptr;
PlatformAbstractionX11::Atoms PlatformAbstractionX11::ourAtoms;

// HELPER FUNCTIONS FOR XLIB

static unsigned long xGetWindowProperty(AWindow& window, Atom property, Atom type, unsigned char** value) {
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty(
        PlatformAbstractionX11::ourDisplay, PlatformAbstractionX11::nativeHandle(window), property, 0,
        std::numeric_limits<long>::max(), false, type, &actualType, &actualFormat, &itemCount, &bytesAfter, value);

    return itemCount;
}

static void xSendEventToWM(AWindow& window, Atom atom, long a, long b, long c, long d, long e) {
    if (!PlatformAbstractionX11::nativeHandle(window))
        return;
    XEvent event = { 0 };
    event.type = ClientMessage;
    event.xclient.window = PlatformAbstractionX11::nativeHandle(window);
    event.xclient.format = 32;   // Data is 32-bit longs
    event.xclient.message_type = atom;
    event.xclient.data.l[0] = a;
    event.xclient.data.l[1] = b;
    event.xclient.data.l[2] = c;
    event.xclient.data.l[3] = d;
    event.xclient.data.l[4] = e;

    XSendEvent(
        PlatformAbstractionX11::ourDisplay, DefaultRootWindow(PlatformAbstractionX11::ourDisplay), False,
        SubstructureNotifyMask | SubstructureRedirectMask, &event);
}

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
            if (d == nullptr)
                return;
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
            // XCloseDisplay(ourDisplay);
            // XFree(ourScreen);
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
                                if (auto x11ctx =
                                        dynamic_cast<RenderingContextX11*>(window->getRenderingContext().get())) {
                                    x11ctx->sync().lo = ev.xclient.data.l[2];
                                    x11ctx->sync().hi = ev.xclient.data.l[3];
                                }
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
                        auto x11ctx = dynamic_cast<RenderingContextX11*>(window->getRenderingContext().get());
                        if (!x11ctx) {
                            break;
                        }
                        count = Xutf8LookupString(
                            (XIC) x11ctx->ic(), (XKeyPressedEvent*) &ev, buf, sizeof(buf), &keysym, &status);

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
                            AUI_NULLSAFE(window->getRenderingContext())->beginResize(*window);
                            window->AViewContainer::setSize(size);
                            AUI_NULLSAFE(window->getRenderingContext())->endResize(*window);
                        }
                        if (auto w = _cast<ACustomWindow>(window)) {
                            AUI_EMIT_FOREIGN(w, dragEnd);

                            // x11 does not send release button event
                            w->AViewContainer::onPointerReleased({ { 0, 0 }, APointerIndex::button(AInput::LBUTTON) });
                        }
                        redrawFlag(*window) = true;

                        auto x11ctx = dynamic_cast<RenderingContextX11*>(window->getRenderingContext().get());
                        if (!x11ctx) {
                            break;
                        }
                        XSyncValue syncValue;
                        XSyncIntsToValue(&syncValue, x11ctx->sync().lo, x11ctx->sync().hi);
                        XSyncSetCounter(ourDisplay, x11ctx->sync().counter, syncValue);

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
                            if (maximized != wasMaximized(*window)) {
                                if (wasMaximized(*window)) {
                                    AUI_EMIT_FOREIGN(window, restored);
                                } else {
                                    AUI_EMIT_FOREIGN(window, maximized);
                                }
                                wasMaximized(*window) = maximized;
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
                        xHandleClipboard(ev);
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

float PlatformAbstractionX11::windowFetchDpiFromSystem(AWindow& window) { return APlatform::getDpiRatio(); }

void PlatformAbstractionX11::windowRestore(AWindow& window) {
    if (PlatformAbstractionX11::ourAtoms.netWmState && PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert &&
        PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz) {
        xSendEventToWM(window,
            PlatformAbstractionX11::ourAtoms.netWmState, 0, PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert,
            PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz, 1, 0);
    }
}

void PlatformAbstractionX11::windowMinimize(AWindow& window) {
    if (!nativeHandle(window))
        return;
    XIconifyWindow(PlatformAbstractionX11::ourDisplay, nativeHandle(window), 0);
}

bool PlatformAbstractionX11::windowIsMinimized(AWindow& window) const {
    if (!nativeHandle(window))
        return false;
    int result = WithdrawnState;
    struct {
        uint32_t state;
        Window icon;
    }* state = NULL;

    if (xGetWindowProperty(window,
            PlatformAbstractionX11::ourAtoms.wmState, PlatformAbstractionX11::ourAtoms.wmState,
            (unsigned char**) &state) >= 2) {
        result = state->state;
    }

    if (state)
        XFree(state);

    return result == IconicState;
}

bool PlatformAbstractionX11::windowIsMaximized(AWindow& window) const {
    if (!nativeHandle(window))
        return false;
    Atom* states;
    unsigned long i;
    bool maximized = false;

    if (!PlatformAbstractionX11::ourAtoms.netWmState || !PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert ||
        !PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz) {
        return maximized;
    }

    const unsigned long count =
        xGetWindowProperty(window, PlatformAbstractionX11::ourAtoms.netWmState, XA_ATOM, (unsigned char**) &states);

    for (i = 0; i < count; i++) {
        if (states[i] == PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert ||
            states[i] == PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz) {
            maximized = true;
            break;
        }
    }

    if (states)
        XFree(states);

    return maximized;
}

void PlatformAbstractionX11::windowMaximize(AWindow& window) {
    if (!nativeHandle(window))
        return;
    // https://github.com/glfw/glfw/blob/master/src/x11_window.c#L2355

    if (!PlatformAbstractionX11::ourAtoms.netWmState || !PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert ||
        !PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz) {
        return;
    }

    XWindowAttributes wa;
    XGetWindowAttributes(PlatformAbstractionX11::ourDisplay, nativeHandle(window), &wa);

    if (wa.map_state == IsViewable) {
        xSendEventToWM(window,
            PlatformAbstractionX11::ourAtoms.netWmState, 1, PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz,
            PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert, 0, 0);
    } else {
        Atom* states = NULL;
        unsigned long count =
            xGetWindowProperty(window, PlatformAbstractionX11::ourAtoms.netWmState, XA_ATOM, (unsigned char**) &states);

        // NOTE: We don't check for failure as this property may not exist yet
        //       and that's fine (and we'll create it implicitly with append)

        Atom missing[2] = {
            PlatformAbstractionX11::ourAtoms.netWmStateMaximizedVert,
            PlatformAbstractionX11::ourAtoms.netWmStateMaximizedHorz
        };
        unsigned long missingCount = 2;

        for (unsigned long i = 0; i < count; i++) {
            for (unsigned long j = 0; j < missingCount; j++) {
                if (states[i] == missing[j]) {
                    missing[j] = missing[missingCount - 1];
                    missingCount--;
                }
            }
        }

        if (states)
            XFree(states);

        if (!missingCount)
            return;

        XChangeProperty(
            PlatformAbstractionX11::ourDisplay, nativeHandle(window), PlatformAbstractionX11::ourAtoms.netWmState,
            XA_ATOM, 32, PropModeAppend, (unsigned char*) missing, missingCount);
    }
    XFlush(PlatformAbstractionX11::ourDisplay);
}

glm::ivec2 PlatformAbstractionX11::windowGetPosition(AWindow& window) const {
    if (!nativeHandle(window))
        return { 0, 0 };
    int x, y;
    Window child;
    XWindowAttributes xwa;
    XTranslateCoordinates(
        PlatformAbstractionX11::ourDisplay, nativeHandle(window), PlatformAbstractionX11::ourScreen->root, 0, 0, &x, &y,
        &child);
    XGetWindowAttributes(PlatformAbstractionX11::ourDisplay, nativeHandle(window), &xwa);

    return { x, y };
}

void PlatformAbstractionX11::windowFlagRedraw(AWindow& window) { redrawFlag(window) = true; }
void PlatformAbstractionX11::windowShow(AWindow& window) {
    if (bool(PlatformAbstractionX11::ourDisplay) && nativeHandle(window)) {
        AThread::current() << [&]() { XMapWindow(PlatformAbstractionX11::ourDisplay, nativeHandle(window)); };
    }
}

void PlatformAbstractionX11::windowSetSize(AWindow& window, glm::ivec2 size) {
    if (!nativeHandle(window))
        return;
    if (!!(window.windowStyle() & WindowStyle::NO_RESIZE)) {
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

        sizehints->min_width = window.getMinimumWidth();
        sizehints->min_height = window.getMinimumHeight();
        sizehints->flags |= PMinSize;

        XSetWMNormalHints(PlatformAbstractionX11::ourDisplay, nativeHandle(window), sizehints);

        XFree(sizehints);
    }
}

void PlatformAbstractionX11::windowSetGeometry(AWindow& window, int x, int y, int width, int height) {
    if (!nativeHandle(window))
        return;
    XMoveWindow(PlatformAbstractionX11::ourDisplay, nativeHandle(window), x, y);
    XResizeWindow(PlatformAbstractionX11::ourDisplay, nativeHandle(window), width, height);
}

void PlatformAbstractionX11::windowSetIcon(AWindow& window, const AImage& image) {}

void PlatformAbstractionX11::windowHide(AWindow& window) {
    if (!nativeHandle(window))
        return;
    XUnmapWindow(PlatformAbstractionX11::ourDisplay, nativeHandle(window));
}

void PlatformAbstractionX11::windowManagerNotifyProcessMessages() {
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
    auto& wm = AWindow::getWindowManager();
    wm.start();
    while (wm.isLoopRunning() && !wm.getWindows().empty()) {
        try {
            xProcessEvent(ev);
        } catch (const AException& e) {
            ALogger::err("AUI") << "Uncaught exception in window proc: " << e;
        }

        AThread::processMessages();
        if (AWindow::isRedrawWillBeEfficient()) {
            for (const auto& window : wm.getWindows()) {
                if (redrawFlag(*window)) {
                    redrawFlag(*window) = false;
                    setCurrentWindow(window.get());
                    wm.watchdog().runOperation([&] { window->redraw(); });
                }
            }
        }

        // [1000 ms timeout] sometimes, leaving an always rerendering window (game) work a long time deadlocks the loop
        // in infinite poll.
        const auto timeout =
            mFastPathNotify || ranges::any_of(wm.getWindows(), [](const _<AWindow>& window) { return redrawFlag(*window); })
                ? 0
                : 1000;
        if (int p = poll(ps, std::size(ps), timeout); p < 0) {
            aui::impl::unix_based::lastErrorToException("eventloop poll failed");
        } else if (p == 0) {
            continue;
        }
        mFastPathNotify = false;
        if (ps[1].revents & POLLIN) {
            char unused[0xff];
            while (read(mNotifyPipe.out(), unused, std::size(unused)) == std::size(unused))
                ;
        }
    }
}

void PlatformAbstractionX11::windowManagerInitNativeWindow(const IRenderingContext::Init& init) {
    for (const auto& graphicsApi : ARenderingContextOptions::get().initializationOrder) {
        try {
            std::visit(aui::lambda_overloaded{
                [](const ARenderingContextOptions::DirectX11&) {
                  throw AException("DirectX is not supported on linux");
                },
                [&](const ARenderingContextOptions::OpenGL& config) {
                  auto context = std::make_unique<OpenGLRenderingContextX11>(config);
                  context->init(init);
                  init.setRenderingContext(std::move(context));
                },
                [&](const ARenderingContextOptions::Software&) {
                  auto context = std::make_unique<SoftwareRenderingContextX11>();
                  context->init(init);
                  init.setRenderingContext(std::move(context));
                },
            }, graphicsApi);
            return;
        } catch (const AException& e) {
            ALogger::warn("AWindowManager") << "Unable to initialize graphics API:" << e;
        }
    }
}

void PlatformAbstractionX11::windowBlockUserInput(AWindow& window, bool blockUserInput) {
}

void PlatformAbstractionX11::windowAllowDragNDrop(AWindow& window) {}

void PlatformAbstractionX11::windowShowTouchscreenKeyboardImpl(AWindow& window) {
}

void PlatformAbstractionX11::windowHideTouchscreenKeyboardImpl(AWindow& window) {
}

void PlatformAbstractionX11::windowMoveToCenter(AWindow& window) {}

void PlatformAbstractionX11::windowAnnounceMinMaxSize(AWindow& window) {
    if (PlatformAbstractionX11::ourDisplay != nullptr) {
        auto sizeHints = aui::ptr::manage_unique(XAllocSizeHints(), XFree);
        sizeHints->flags = PMinSize | PMaxSize;
        sizeHints->min_width = window.getMinimumWidth();
        sizeHints->min_height = window.getMinimumHeight();
        sizeHints->max_width = window.getMaxSize().x;
        sizeHints->max_height = window.getMaxSize().y;
        XSetWMNormalHints(PlatformAbstractionX11::ourDisplay, nativeHandle(window), sizeHints.get());
    }
}

void PlatformAbstractionX11::init() {}
