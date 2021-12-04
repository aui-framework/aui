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


#include "AUI/GL/gl.h"
#include "AUI/GL/GLDebug.h"
#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/Render.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Util/ARandom.h"
#include "AUI/GL/State.h"
#include "AUI/Thread/AThread.h"
#include "AUI/Render/OpenGLRenderer.h"
#include "AUI/Platform/Platform.h"
#include "AUI/Platform/ACustomWindow.h"

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>
#include <AUI/Traits/arrays.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>

#include <X11/extensions/sync.h>

Display* gDisplay = nullptr;
Screen* gScreen;
int gScreenId;

int xerrorhandler(Display* dsp, XErrorEvent* error) {
    if (gDisplay == dsp) {
        char errorstring[0x100];
        XGetErrorText(dsp, error->error_code, errorstring, sizeof(errorstring));
        printf("X Error: %s\n", errorstring);
    }
    return 0;
}

struct {
    Atom wmProtocols;
    Atom wmDeleteWindow;
    Atom wmHints;
    Atom wmState;
    Atom netWmState;
    Atom netWmStateMaximizedVert;
    Atom netWmStateMaximizedHorz;
    Atom clipboard;
    Atom utf8String;
    Atom textPlain;
    Atom textPlainUtf8;
    Atom auiClipboard;
    Atom incr;
    Atom targets;
    Atom netWmSyncRequest;
    Atom netWmSyncRequestCounter;


    void init() {
        wmProtocols = XInternAtom(gDisplay, "WM_PROTOCOLS", False);
        wmDeleteWindow = XInternAtom(gDisplay, "WM_DELETE_WINDOW", False);
        wmHints = XInternAtom(gDisplay, "_MOTIF_WM_HINTS", true);
        wmState = XInternAtom(gDisplay, "WM_STATE", true);
        netWmState = XInternAtom(gDisplay, "_NET_WM_STATE", false);
        netWmStateMaximizedVert = XInternAtom(gDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", false);
        netWmStateMaximizedHorz = XInternAtom(gDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
        clipboard = XInternAtom(gDisplay, "CLIPBOARD", False);
        utf8String = XInternAtom(gDisplay, "UTF8_STRING", False);
        textPlain = XInternAtom(gDisplay, "text/plain", False);
        textPlainUtf8 = XInternAtom(gDisplay, "text/plain;charset=utf-8", False);
        auiClipboard = XInternAtom(gDisplay, "AUI_CLIPBOARD", False);
        incr = XInternAtom(gDisplay, "INCR", False);
        targets = XInternAtom(gDisplay, "TARGETS", False);
        netWmSyncRequest = XInternAtom(gDisplay, "_NET_WM_SYNC_REQUEST", False);
        netWmSyncRequestCounter = XInternAtom(gDisplay, "_NET_WM_SYNC_REQUEST_COUNTER", False);
    }

} gAtoms;

void ensureXLibInitialized() {
    struct DisplayInstance {

    public:
        DisplayInstance() {
            gDisplay = XOpenDisplay(nullptr);
            XSetErrorHandler(xerrorhandler);
            gScreen = DefaultScreenOfDisplay(gDisplay);
            gScreenId = DefaultScreen(gDisplay);
            gAtoms.init();
        }

        ~DisplayInstance() {
            //XCloseDisplay(gDisplay);
            //XFree(gScreen);

        }
    };
    static DisplayInstance display;
}


struct painter {
private:

public:
    static thread_local bool painting;

    painter(Window window) {
        glXMakeCurrent(gDisplay, window, AWindow::context.context);
    }

    ~painter() {

    }
};



#if !(AUI_PLATFORM_APPLE)
thread_local bool painter::painting = false;
#endif


AWindow::Context::~Context() {
    //glXDestroyContext(gDisplay, context);
}
using namespace std::chrono;
using namespace std::chrono_literals;
static auto _gLastFrameTime = 0ms;
extern unsigned char stencilDepth;

void AWindow::redraw() {
    if (mUpdateLayoutFlag) {
        mUpdateLayoutFlag = false;
        updateLayout();
    }

    {

        // fps restriction
        {
            auto now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
            auto delta = now - _gLastFrameTime;
            // restriction 16ms = up to 60 frames per second
            const auto FRAME_DURATION = 16ms;

            if (FRAME_DURATION > delta) {
                std::this_thread::sleep_for(FRAME_DURATION - delta);
            }
            _gLastFrameTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
        }

        painter p(mHandle);

        GL::State::activeTexture(0);
        GL::State::bindTexture(GL_TEXTURE_2D, 0);
        GL::State::bindVertexArray(0);
        GL::State::useProgram(0);

        Render::setWindow(this);
        glViewport(0, 0, getWidth(), getHeight());

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glEnable(GL_MULTISAMPLE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // stencil
        glClearStencil(0);
        glStencilMask(0xff);
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_STENCIL_BUFFER_BIT);
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0x00);
        stencilDepth = 0;
        glStencilFunc(GL_EQUAL, 0, 0xff);

        doDrawWindow();

        glXSwapBuffers(gDisplay, mHandle);
    }

    emit redrawn();
}



void AWindow::windowNativePreInit(const AString& name, int width, int height, AWindow* parent, WindowStyle ws) {
    mWindowTitle = name;
    mParentWindow = parent;

    currentWindowStorage() = this;

    connect(closed, this, &AWindow::close);

    do_once {
        if (!XSupportsLocale() || XSetLocaleModifiers("@im=none") == NULL) {
            throw AException("Your X server does not support locales.");
        }
    }
    ensureXLibInitialized();

    static XVisualInfo* vi;
    static XSetWindowAttributes swa;
    static XIM im;
    static XIMStyles *styles;

    if (context.context == nullptr) {
        GLint att[] = {GLX_X_RENDERABLE, True, // 1
                       GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, // 3
                       GLX_RENDER_TYPE, GLX_RGBA_BIT, // 5
                       GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR, // 7
                       GLX_RED_SIZE, 8, // 9
                       GLX_GREEN_SIZE, 8, // 11
                       GLX_BLUE_SIZE, 8, // 13
                       GLX_ALPHA_SIZE, 8, // 15
                       GLX_DEPTH_SIZE, 24,
                       GLX_STENCIL_SIZE, 8,
                       GLX_DOUBLEBUFFER, true,
                       GLX_STENCIL_SIZE, 8,
                       GLX_SAMPLE_BUFFERS  , 1,
                       GLX_SAMPLES         , 16,
                       None};

        int fbcount;
        GLXFBConfig* fbc = glXChooseFBConfig(gDisplay, DefaultScreen(gDisplay), att, &fbcount);

        if (fbc == nullptr || fbcount <= 0) {
            // try to reduce system requirements
            size_t indexToReduce = aui::array_length(att) - 2;
            do {
                ALogger::warn("[OpenGL compatibility] Reduced OpenGL requirements: pass {}"_format((aui::array_length(att) - indexToReduce) / 2 - 1));
                att[indexToReduce] = 0;
                indexToReduce -= 2;
                fbc = glXChooseFBConfig(gDisplay, DefaultScreen(gDisplay), att, &fbcount);
            } while ((fbc == nullptr || fbcount <= 0) && indexToReduce > 13); // up to GLX_BLUE_SIZE

            if (fbc == nullptr || fbcount <= 0) {
                // try to disable rgba.
                att[5] = 0;
                ALogger::warn("[OpenGL compatibility] Disabled RGBA");
                fbc = glXChooseFBConfig(gDisplay, DefaultScreen(gDisplay), att, &fbcount);

                if (fbc == nullptr || fbcount <= 0) {
                    // use default attribs
                    ALogger::warn("[OpenGL compatibility] Using default attribs");
                    glXChooseFBConfig(gDisplay, DefaultScreen(gDisplay), nullptr, &fbcount);
                    if (fbc == nullptr || fbcount <= 0) {
                        // giving up.
                        ALogger::err("[OpenGL compatibility] System hardware is not supported. Giving up.");
                        exit(-1);
                    }
                }
            }
        }

        // Pick the FB config/visual with the most samples per pixel
        int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = std::numeric_limits<int>::max();

        int i;
        for (i = 0; i < fbcount; ++i) {
            vi = glXGetVisualFromFBConfig(gDisplay, fbc[i]);
            if (vi) {
                int samp_buf, samples;
                glXGetFBConfigAttrib(gDisplay, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
                glXGetFBConfigAttrib(gDisplay, fbc[i], GLX_SAMPLES, &samples);

                if (best_fbc < 0 || samp_buf && samples > best_num_samp)
                    best_fbc = i, best_num_samp = samples;
                if (worst_fbc < 0 || !samp_buf || samples < worst_num_samp)
                    worst_fbc = i, worst_num_samp = samples;
            }
            XFree(vi);
        }

        GLXFBConfig bestFbc = fbc[best_fbc];

        // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
        XFree(fbc);

        // Get a visual
        vi = glXGetVisualFromFBConfig(gDisplay, bestFbc);
        auto cmap = XCreateColormap(gDisplay, gScreen->root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask
                | PointerMotionMask | StructureNotifyMask | PropertyChangeMask | StructureNotifyMask;
        context.context = glXCreateContext(gDisplay, vi, nullptr, true);

        im = XOpenIM(gDisplay, NULL, NULL, NULL);
        if (im == NULL) {
            throw AException("Could not open input method");
        }

        if (XGetIMValues(im, XNQueryInputStyle, &styles, NULL)) {
            throw AException("XIM Can't get styles");
        }
    }
    mHandle = XCreateWindow(gDisplay,
                            gScreen->root,
                            0, 0,
                            width, height,
                            0,
                            vi->depth,
                            InputOutput,
                            vi->visual,
                            CWColormap | CWEventMask | CWCursor, &swa);

    // XSync
    {
        XSyncValue value;
        XSyncIntToValue(&value, 0);
        mXsyncRequestCounter.counter = XSyncCreateCounter(gDisplay, value);
        XChangeProperty(gDisplay,
                        mHandle,
                        gAtoms.netWmSyncRequestCounter,
                        XA_CARDINAL,
                        32,
                        PropModeReplace,
                        (const unsigned char*)&mXsyncRequestCounter.counter, 1);

    }

    mIC = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, mHandle, NULL);
    if (mIC == NULL) {
        throw AException("Could not get IC");
    }
    XSetICFocus(mIC);


    XMapWindow(gDisplay, mHandle);

    auto title = mWindowTitle.toStdString();
    XStoreName(gDisplay, mHandle, title.c_str());
    XChangeProperty(gDisplay, mHandle, XInternAtom(gDisplay, "_NET_WM_NAME", false),
                    XInternAtom(gDisplay, "UTF8_STRING", false), 8, PropModeReplace,
                    reinterpret_cast<const unsigned char*>(title.c_str()), title.length());

    XSetWMProtocols(gDisplay, mHandle, &gAtoms.wmDeleteWindow, 1);


    glXMakeCurrent(gDisplay, mHandle, context.context);

    if (!glewExperimental) {
        ALogger::info((const char*) glGetString(GL_VERSION));
        ALogger::info((const char*) glGetString(GL_VENDOR));
        ALogger::info((const char*) glGetString(GL_RENDERER));
        ALogger::info((const char*) glGetString(GL_EXTENSIONS));
        glewExperimental = true;
        if (glewInit() != GLEW_OK) {
            throw AException("glewInit failed");
        }
        ALogger::info("OpenGL context is ready");
        Render::setRenderer(std::make_unique<OpenGLRenderer>());
    }

    if (parent) {
        XSetTransientForHint(gDisplay, mHandle, parent->mHandle);
    }

#if defined(_DEBUG)
    GL::setupDebug();
#endif
    //assert(glGetError() == 0);

    updateDpi();
    Render::setWindow(this);

    checkForStencilBits();

    setWindowStyle(ws);

}

AWindow::~AWindow() {
    XDestroyWindow(gDisplay, mHandle);
}


bool AWindow::isRenderingContextAcquired() {
    return painter::painting;
}

void AWindow::quit() {
    getWindowManager().mWindows.remove(shared_from_this());

    XUnmapWindow(gDisplay, mHandle);

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
}

void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;

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
        XChangeProperty(gDisplay, mHandle, gAtoms.wmHints, gAtoms.wmHints, 32, PropModeReplace,
                        (unsigned char *)&hints, 5);
    }
}

void AWindow::updateDpi() {
    emit dpiChanged;

    mDpiRatio = Platform::getDpiRatio();
    onDpiChanged();
}

void AWindow::restore() {
    if (gAtoms.netWmState &&
        gAtoms.netWmStateMaximizedVert &&
        gAtoms.netWmStateMaximizedHorz)
        {
            xSendEventToWM(gAtoms.netWmState,
                           0,
                           gAtoms.netWmStateMaximizedVert,
                           gAtoms.netWmStateMaximizedHorz,
                           1, 0);
        }
}

void AWindow::minimize() {
    XIconifyWindow(gDisplay, mHandle, 0);
}

bool AWindow::isMinimized() const {
    int result = WithdrawnState;
    struct {
        uint32_t state;
        Window icon;
    } *state = NULL;

    if (xGetWindowProperty(gAtoms.wmState, gAtoms.wmState, (unsigned char**) &state) >= 2)
    {
        result = state->state;
    }

    if (state)
        XFree(state);

    return result == IconicState;
}


bool AWindow::isMaximized() const {
    Atom* states;
    unsigned long i;
    bool maximized = false;

    if (!gAtoms.netWmState ||
        !gAtoms.netWmStateMaximizedVert ||
        !gAtoms.netWmStateMaximizedHorz)
    {
        return maximized;
    }

    const unsigned long count = xGetWindowProperty(gAtoms.netWmState, XA_ATOM, (unsigned char**) &states);

    for (i = 0;  i < count;  i++)
    {
        if (states[i] == gAtoms.netWmStateMaximizedVert ||
            states[i] == gAtoms.netWmStateMaximizedHorz)
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
    // https://github.com/glfw/glfw/blob/master/src/x11_window.c#L2355

    if (!gAtoms.netWmState ||
        !gAtoms.netWmStateMaximizedVert ||
        !gAtoms.netWmStateMaximizedHorz)
    {
        return;
    }

    XWindowAttributes wa;
    XGetWindowAttributes(gDisplay, mHandle, &wa);

    if (wa.map_state == IsViewable) {
        xSendEventToWM(gAtoms.netWmState, 1, gAtoms.netWmStateMaximizedHorz, gAtoms.netWmStateMaximizedVert, 0, 0);
    } else {

        Atom* states = NULL;
        unsigned long count =
                xGetWindowProperty(gAtoms.netWmState,
                                          XA_ATOM,
                                          (unsigned char**) &states);

        // NOTE: We don't check for failure as this property may not exist yet
        //       and that's fine (and we'll create it implicitly with append)

        Atom missing[2] =
                {
                        gAtoms.netWmStateMaximizedVert,
                        gAtoms.netWmStateMaximizedHorz
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

        XChangeProperty(gDisplay, mHandle,
                        gAtoms.netWmState, XA_ATOM, 32,
                        PropModeAppend,
                        (unsigned char*) missing,
                        missingCount);
    }
    XFlush(gDisplay);
}

glm::ivec2 AWindow::getWindowPosition() const {
    int x, y;
    Window child;
    XWindowAttributes xwa;
    XTranslateCoordinates(gDisplay, mHandle, gScreen->root, 0, 0, &x, &y, &child);
    XGetWindowAttributes(gDisplay, mHandle, &xwa);

    return {x, y};
}


void AWindow::flagRedraw() {
    mRedrawFlag = true;
}


void AWindow::setSize(int width, int height) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, width, height);

    if (!!(mWindowStyle & WindowStyle::NO_RESIZE)) {
        // we should set min size and max size the same as current size
        XSizeHints* sizehints = XAllocSizeHints();
        long userhints;

        XGetWMNormalHints(gDisplay, mHandle, sizehints, &userhints);

        sizehints->min_width = sizehints->min_width = sizehints->max_width = sizehints->base_width = width;
        sizehints->min_height = sizehints->min_height = sizehints->max_height = sizehints->base_height = height;
        sizehints->flags |= PMinSize | PMaxSize;

        XSetWMNormalHints(gDisplay, mHandle, sizehints);

        XFree(sizehints);
    } else {
        XSizeHints* sizehints = XAllocSizeHints();
        long userhints;

        XGetWMNormalHints(gDisplay, mHandle, sizehints, &userhints);

        sizehints->min_width = getMinimumWidth();
        sizehints->min_height = getMinimumHeight();
        sizehints->flags |= PMinSize;

        XSetWMNormalHints(gDisplay, mHandle, sizehints);

        XFree(sizehints);
    }
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize(width, height);

    XMoveWindow(gDisplay, mHandle, x, y);
    XResizeWindow(gDisplay, mHandle, width, height);
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
    XUnmapWindow(gDisplay, mHandle);
}

// HELPER FUNCTIONS FOR XLIB

unsigned long AWindow::xGetWindowProperty(Atom property, Atom type, unsigned char** value) const {
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty(gDisplay, mHandle, property, 0, std::numeric_limits<long>::max(), false, type, &actualType,
                       &actualFormat, &itemCount, &bytesAfter, value);

    return itemCount;
}

void AWindow::xSendEventToWM(Atom atom, long a, long b, long c, long d, long e) const {
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

    XSendEvent(gDisplay, DefaultRootWindow(gDisplay),
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
        while (XPending(gDisplay)) {
            XNextEvent(gDisplay, &ev);
            _<AWindow> window;
            switch (ev.type) {
                case ClientMessage: {
                    if (ev.xclient.message_type == gAtoms.wmProtocols) {
                        auto window = locateWindow(ev.xclient.window);
                        if(ev.xclient.data.l[0] == gAtoms.wmDeleteWindow) {
                            // close button clicked
                            window->onCloseButtonClicked();
                        } else if (ev.xclient.data.l[0] == gAtoms.netWmSyncRequest) {
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
                    count = Xutf8LookupString(window->mIC, (XKeyPressedEvent*) &ev, buf, sizeof(buf), &keysym,
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
                    window = locateWindow(ev.xkey.window);
                    window->onKeyUp(AInput::fromNative(ev.xkey.keycode));
                    break;

                case ConfigureNotify: {
                    window = locateWindow(ev.xconfigure.window);
                    glm::ivec2 size = {ev.xconfigure.width, ev.xconfigure.height};
                    if (size.x >= 10 && size.y >= 10 && size != window->getSize())
                        window->AViewContainer::setSize(size.x, size.y);
                    if (auto w = _cast<ACustomWindow>(window)) {
                        w->handleXConfigureNotify();
                    }
                    window->mRedrawFlag = false;
                    window->redraw();

                    XSyncValue syncValue;
                    XSyncIntsToValue(&syncValue,
                                     window->mXsyncRequestCounter.lo,
                                     window->mXsyncRequestCounter.hi);
                    XSyncSetCounter(gDisplay, window->mXsyncRequestCounter.counter, syncValue);

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
                                                   (AInput::Key) (AInput::LButton + ev.xbutton.button - 1));
                            break;
                        case 4: // wheel down
                            window->onMouseWheel({ev.xbutton.x, ev.xbutton.y}, -120);
                            break;
                        case 5: // wheel up
                            window->onMouseWheel({ev.xbutton.x, ev.xbutton.y}, 120);
                            break;
                    }
                    break;
                }
                case ButtonRelease: {
                    if (ev.xbutton.button < 4) {
                        window = locateWindow(ev.xbutton.window);
                        window->onMouseReleased({ev.xbutton.x, ev.xbutton.y},
                                                (AInput::Key) (AInput::LButton + ev.xbutton.button - 1));
                    }
                    break;
                }

                case PropertyNotify: {
                    window = locateWindow(ev.xproperty.window);
                    if (ev.xproperty.atom == gAtoms.netWmState) {
                        auto maximized = window->isMaximized();
                        if (maximized != window->mWasMaximized) {
                            apply(window, {
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


                    char* targetName = XGetAtomName(gDisplay, ev.xselectionrequest.target);
                    char* propertyName = XGetAtomName(gDisplay, ev.xselectionrequest.property);
                    ALogger::info("{}: {}"_format(targetName, propertyName));
                    XFree(targetName);
                    XFree(propertyName);
                    if (ev.xselectionrequest.target == gAtoms.utf8String ||
                        ev.xselectionrequest.target == gAtoms.textPlain ||
                        ev.xselectionrequest.target == gAtoms.textPlainUtf8) { // check for UTF8_STRING
                        XChangeProperty(gDisplay,
                                        ev.xselectionrequest.requestor,
                                        ev.xselectionrequest.property,
                                        ev.xselectionrequest.target,
                                        8,
                                        PropModeReplace,
                                        (unsigned char*) mXClipboardText.c_str(),
                                        mXClipboardText.length());
                    } else if (ev.xselectionrequest.target == gAtoms.targets) { // data type request
                        Atom atoms[] = {
                                XInternAtom(gDisplay, "TIMESTAMP", false),
                                XInternAtom(gDisplay, "TARGETS", false),
                                XInternAtom(gDisplay, "SAVE_TARGETS", false),
                                XInternAtom(gDisplay, "MULTIPLE", false),
                                XInternAtom(gDisplay, "STRING", false),
                                XInternAtom(gDisplay, "UTF8_STRING", false),
                                XInternAtom(gDisplay, "text/plain", false),
                                XInternAtom(gDisplay, "text/plain;charset=utf-8", false),
                        };
                        XChangeProperty(gDisplay,
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

                    XSendEvent(gDisplay, ev.xselectionrequest.requestor, True, NoEventMask, (XEvent *)&ssev);
                    break;
                }
            }
        }

        {
            std::unique_lock lock(mXNotifyLock);
            mXNotifyCV.wait_for(lock, std::chrono::microseconds(500));
        }
        AThread::current()->processMessages();
        if (AWindow::isRedrawWillBeEfficient()) {
            for (auto &window : mWindows) {
                if (window->mRedrawFlag) {
                    window->mRedrawFlag = false;
                    window->redraw();
                }
            }
        }
    } catch(NotFound e) {

    }
}

AString AWindowManager::xClipboardPasteImpl() {

    auto owner = XGetSelectionOwner(gDisplay, gAtoms.clipboard);
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

    XConvertSelection(gDisplay, gAtoms.clipboard, gAtoms.utf8String, gAtoms.auiClipboard, nativeHandle,
                      CurrentTime);

    XEvent ev;
    for (;;)
    {
        XNextEvent(gDisplay, &ev);
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

                XGetWindowProperty(gDisplay, nativeHandle, gAtoms.auiClipboard, 0, 0, False, AnyPropertyType,
                                   &type, &di, &dul, &size, &prop_ret);
                XFree(prop_ret);

                if (type == gAtoms.incr)
                {
                    ALogger::warn("Clipboard data is too large and INCR mechanism not implemented");
                    return {};
                }

                XGetWindowProperty(gDisplay, nativeHandle, gAtoms.auiClipboard, 0, size, False, AnyPropertyType,
                                   &da, &di, &dul, &dul, &prop_ret);
                AString clipboardData = (const char*)prop_ret;
                XFree(prop_ret);

                XDeleteProperty(gDisplay, nativeHandle, gAtoms.auiClipboard);
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
    XSetSelectionOwner(gDisplay, gAtoms.clipboard, auiWindow->mHandle, CurrentTime);
}
