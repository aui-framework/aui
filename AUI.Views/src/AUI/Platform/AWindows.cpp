
#include "AUI/GL/gl.h"
#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/Render.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Util/ARandom.h"
#include "AUI/GL/State.h"
#include "AUI/Thread/AThread.h"
#include "Platform.h"
#include "AMessageBox.h"
#include "AWindowManager.h"

#include <chrono>
#include <AUI/Logging/ALogger.h>

AWindow::Context AWindow::context = {};

#if defined(_WIN32)

#include <GL/wglew.h>

struct painter {
private:
    HWND mHandle;
    PAINTSTRUCT mPaint;

public:
    HDC mHdc;
    static thread_local bool painting;

    painter(HWND handle) :
            mHandle(handle) {
        assert(!painting);
        painting = true;
        mHdc = BeginPaint(mHandle, &mPaint);
        bool ok = wglMakeCurrent(mHdc, AWindow::context.hrc);
        assert(ok);
    }

    ~painter() {
        assert(painting);
        painting = false;
        //SwapBuffers(hdc);
        bool ok = wglMakeCurrent(mHdc, nullptr);
        assert(ok);
        EndPaint(mHandle, &mPaint);
    }
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    AWindow* window = reinterpret_cast<AWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (window)
        return window->winProc(hwnd, uMsg, wParam, lParam);

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT AWindow::winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
#define POS glm::ivec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))

    static glm::ivec2 lastWindowSize;

    currentWindowStorage() = this;

    switch (uMsg) {
        /*
    case WM_SIZING:
        {
        auto rect = reinterpret_cast<RECT*>(lParam);
        }
        break;*/

        case WM_SETFOCUS:
            onFocusAcquired();
            return 0;

        case WM_KILLFOCUS:
            onFocusLost();
            return 0;

        case WM_PAINT: {
            redraw();

            return 0;
        }
        case WM_KEYDOWN:
            if (lParam & (1 << 30)) {
                // автоповторение
                onKeyRepeat(AInput::fromNative(wParam));
            } else {
                onKeyDown(AInput::fromNative(wParam));
            }
            return 0;

        case WM_KEYUP:
            onKeyUp(AInput::fromNative(wParam));
            return 0;

        case WM_CHAR: {
            onCharEntered(wParam);
            return 0;
        }

        case WM_SIZE:
            emit resized(LOWORD(lParam), HIWORD(lParam));
            AViewContainer::setSize(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_MOUSEMOVE:
            onMouseMove(POS);

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_LEAVE;
            TrackMouseEvent(&tme);

            return 0;

        case WM_MOUSELEAVE:
            onMouseLeave();
            return 0;

        case WM_SETCURSOR: {
            if (LOWORD(lParam) == HTCLIENT) {
                switch (mCursor) {
                    case ACursor::DEFAULT: {
                        static auto cursor = LoadCursor(nullptr, IDC_ARROW);
                        SetCursor(cursor);
                        return true;
                    }
                    case ACursor::POINTER: {
                        static auto cursor = LoadCursor(nullptr, IDC_PERSON);
                        SetCursor(cursor);
                        return true;
                    }
                    case ACursor::TEXT: {
                        static auto cursor = LoadCursor(nullptr, IDC_IBEAM);
                        SetCursor(cursor);
                        return true;
                    }
                }
            }
            break;
        }


        case WM_CLOSE:
            emit closed();
            return 0;

        case WM_LBUTTONDOWN:
            onMousePressed(POS, AInput::LButton);
            SetCapture(mHandle);
            return 0;
        case WM_LBUTTONUP: {
            auto context = acquireTemporaryRenderingContext();
            onMouseReleased(POS, AInput::LButton);
            ReleaseCapture();
            return 0;
        }
        case WM_RBUTTONDOWN:
            onMousePressed(POS, AInput::RButton);
            SetCapture(mHandle);
            return 0;
        case WM_RBUTTONUP:
            onMouseReleased(POS, AInput::RButton);
            ReleaseCapture();
            return 0;

        case WM_LBUTTONDBLCLK:
            onMouseDoubleClicked(POS, AInput::LButton);
            return 0;

        case WM_DPICHANGED:
            updateDpi();
            return 0;

        case WM_GETMINMAXINFO: {
            MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(lParam);

            RECT r = {0, 0, getMinimumWidth(), getMinimumHeight()};
            AdjustWindowRectEx(&r, GetWindowLongPtr(mHandle, GWL_STYLE), false, GetWindowLongPtr(mHandle, GWL_EXSTYLE));
            info->ptMinTrackSize.x = r.right - r.left;
            info->ptMinTrackSize.y = r.bottom - r.top;
        }
            return 0;
        case WM_ERASEBKGND:
            return TRUE;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);

#undef GET_X_LPARAM
#undef GET_Y_LPARAM
#undef POS
}
#elif defined(ANDROID)

#include <AUI/Platform/OSAndroid.h>

struct painter {
private:
    jobject mHandle;

public:
    static thread_local bool painting;

    painter(jobject handle) :
            mHandle(handle) {
        assert(!painting);
        painting = true;
    }

    ~painter() {
        assert(painting);
        painting = false;
    }
};
#else

Display* gDisplay;
Screen* gScreen;
int gScreenId;


struct DisplayLock {
    DisplayLock() {
        XLockDisplay(gDisplay);
    }
    ~DisplayLock() {
        XUnlockDisplay(gDisplay);
    }
};

struct painter {
private:

public:
    static thread_local bool painting;

    painter(Window window) {
    }

    ~painter() {
    }
};

int xerrorhandler(Display* dsp, XErrorEvent* error) {
    if (gDisplay == dsp) {
        char errorstring[0x100];
        XGetErrorText(dsp, error->error_code, errorstring, sizeof(errorstring));
        printf("X Error: %s\n", errorstring);
    }
    return 0;
}

#endif


thread_local bool painter::painting = false;


AWindow*& AWindow::currentWindowStorage() {
    thread_local AWindow* threadLocal = nullptr;
    static AWindow* global = nullptr;
    if (threadLocal)
        return threadLocal;
    return global;
}

AWindow::Context::~Context() {
#if defined(_WIN32)
    wglDeleteContext(hrc);
#elif defined(ANDROID)
#else
    glXDestroyContext(gDisplay, context);
#endif
}

void AWindow::onClosed() {
    quit();
}

void AWindow::doDrawWindow() {
    render();
}

AWindow::AWindow(const AString& name, int width, int height, AWindow* parent) :
        mWindowTitle(name),
        mParentWindow(parent) {
    AVIEW_CSS;

    currentWindowStorage() = this;

    connect(closed, this, &AWindow::close);

#if defined(_WIN32)
    // CREATE WINDOW
    WNDCLASSEX winClass;

    mInst = GetModuleHandle(nullptr);

    ARandom r;
    for (;;) {
        mWindowClass = "AUI-" + AString::number(r.nextInt());
        winClass.lpszClassName = mWindowClass.c_str();
        winClass.cbSize = sizeof(WNDCLASSEX);
        winClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        winClass.lpfnWndProc = WindowProc;
        winClass.hInstance = mInst;
        //winClass.hIcon = LoadIcon(mInst, (LPCTSTR)101);
        //winClass.hIconSm = LoadIcon(mInst, (LPCTSTR)101);
        winClass.hIcon = 0;
        winClass.hIconSm = 0;
        winClass.hbrBackground = nullptr;
        winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        winClass.lpszMenuName = mWindowClass.c_str();
        winClass.cbClsExtra = 0;
        winClass.cbWndExtra = 0;
        if (RegisterClassEx(&winClass)) {
            break;
        }
    }

    DWORD style = WS_OVERLAPPEDWINDOW;

    mHandle = CreateWindowEx(WS_EX_DLGMODALFRAME, mWindowClass.c_str(), name.c_str(), style,
                             GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2,
                             GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2, width, height,
                             parent != nullptr ? parent->mHandle : nullptr, nullptr, mInst, nullptr);

    SetWindowLongPtr(mHandle, GWLP_USERDATA, reinterpret_cast<LONG>(this));

    if (mParentWindow) {
        EnableWindow(mParentWindow->mHandle, false);
    }

    mDC = GetDC(mHandle);

    // INITIALIZE OPENGL
    static PIXELFORMATDESCRIPTOR pfd;
    static int pxf;
    if (context.hrc == nullptr) {
        ALogger::info("Creating OpenGL context...");
        struct FakeWindow {
            HWND mHwnd;
            HDC mDC;

            explicit FakeWindow(HWND hwnd) : mHwnd(hwnd) {
                mDC = GetDC(mHwnd);
            }

            ~FakeWindow() {
                ReleaseDC(mHwnd, mDC);
                DestroyWindow(mHwnd);
            }
        } fakeWindow(CreateWindowEx(WS_EX_DLGMODALFRAME, mWindowClass.c_str(), name.c_str(), style,
                                    GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2,
                                    GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2, width, height,
                                    parent != nullptr ? parent->mHandle : nullptr, nullptr, mInst, nullptr));

        memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_SUPPORT_COMPOSITION;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 8;
        pfd.cStencilBits = 8;
        size_t iPixelFormat = ChoosePixelFormat(fakeWindow.mDC, &pfd);
        DescribePixelFormat(fakeWindow.mDC, iPixelFormat, sizeof(pfd), &pfd);
        SetPixelFormat(fakeWindow.mDC, iPixelFormat, &pfd);

        // инициализация контекста
        context.hrc = wglCreateContext(fakeWindow.mDC);
        wglMakeCurrent(fakeWindow.mDC, context.hrc);

        ALogger::info("Initialized temporary GL context");

        if (!glewExperimental) {
            ALogger::info((const char*) glGetString(GL_VERSION));
            ALogger::info((const char*) glGetString(GL_VENDOR));
            ALogger::info((const char*) glGetString(GL_RENDERER));
            glewExperimental = true;
            if (glewInit() != GLEW_OK) {
                AMessageBox::show(nullptr, "OpenGL", "Could not initialize OpenGL context");
                throw std::runtime_error("glewInit failed");
            }
        }
        bool k;

        auto makeContext = [&](unsigned i) {
            const int iPixelFormatAttribList[] =
                    {
                            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                            WGL_COLOR_BITS_ARB, 24,
                            WGL_ALPHA_BITS_ARB, 8,
                            WGL_DEPTH_BITS_ARB, 24,
                            WGL_STENCIL_BITS_ARB, 8,
                            WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
                            WGL_SAMPLES_ARB, i,
                            0
                    };

            UINT iNumFormats;
            wglChoosePixelFormatARB(mDC, iPixelFormatAttribList, nullptr, 1, &pxf, &iNumFormats);
            assert(iNumFormats);
            DescribePixelFormat(mDC, pxf, sizeof(pfd), &pfd);
            k = SetPixelFormat(mDC, pxf, &pfd);
        };
        makeContext(16);
        if (!k) {
            ALogger::info("Could not set pixel format with MSAA; trying to do the same but without MSAA");
            makeContext(0);
            if (!k) {
                ALogger::info("Could not set pixel format even without MSAA. Giving up.");
                throw AException("Could set pixel format");
            } else {
                ALogger::info("Successfully set pixel format without MSAA");
            }
        } else {
            ALogger::info("Successfully set pixel format with MSAA");
        }
        GLint attribs[] =
                {
                        WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
                        WGL_CONTEXT_MINOR_VERSION_ARB, 0,
                        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                        0
                };
        {
            HGLRC CompHRC = wglCreateContextAttribsARB(mDC, nullptr, attribs);
            if (CompHRC && wglMakeCurrent(mDC, CompHRC)) {
                wglDeleteContext(context.hrc);
                context.hrc = CompHRC;
            } else
                throw std::runtime_error("Failed to create OpenGL 2.0 context");
        }
        ALogger::info("OpenGL context is ready");

        //wglMakeCurrent(mDC, nullptr);
    } else {
        bool k = SetPixelFormat(mDC, pxf, &pfd);
        assert(k);
    }

    wglMakeCurrent(mDC, context.hrc);

#elif defined(ANDROID)
    ALogger::info((const char*) glGetString(GL_VERSION));
    ALogger::info((const char*) glGetString(GL_VENDOR));
    ALogger::info((const char*) glGetString(GL_RENDERER));
#else
    static struct once {
        once() {
            if (!XSupportsLocale() || XSetLocaleModifiers("@im=none") == NULL) {
                throw AException("Your X server does not support locales.");
            }
        }
    } once;
    struct DisplayInstance {

    public:
        DisplayInstance() {
            gDisplay = XOpenDisplay(nullptr);
            XSetErrorHandler(xerrorhandler);
            gScreen = DefaultScreenOfDisplay(gDisplay);
            gScreenId = DefaultScreen(gDisplay);

            if (!XInitThreads()) {
                throw AException("Your X server does not support multithreading; abort");
            }
        }

        ~DisplayInstance() {
            XFree(gScreen);

            XCloseDisplay(gDisplay);
        }
    };
    static DisplayInstance display;
    DisplayLock displayLock;

    static XVisualInfo* vi;
    static XSetWindowAttributes swa;
    static XIM im;
    static XIMStyles *styles;

    if (context.context == nullptr) {
        GLint att[] = {GLX_X_RENDERABLE, True,
                       GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
                       GLX_RENDER_TYPE, GLX_RGBA_BIT,
                       GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
                       GLX_RED_SIZE, 8,
                       GLX_GREEN_SIZE, 8,
                       GLX_BLUE_SIZE, 8,
                       GLX_ALPHA_SIZE, 8,
                       GLX_DEPTH_SIZE, 24,
                       GLX_STENCIL_SIZE, 8,
                       GLX_DOUBLEBUFFER, true,
                       GLX_STENCIL_SIZE, 8,
                       GLX_SAMPLE_BUFFERS  , 1,
                       GLX_SAMPLES         , 16,
                       None};

        int fbcount;
        GLXFBConfig* fbc = glXChooseFBConfig(gDisplay, DefaultScreen(gDisplay), att, &fbcount);

        // Pick the FB config/visual with the most samples per pixel
        int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

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
                | PointerMotionMask | StructureNotifyMask;
        context.context = glXCreateContext(gDisplay, vi, nullptr, true);

        im = XOpenIM(gDisplay, NULL, NULL, NULL);
        if (im == NULL) {
            throw AException("Could not open input method");
        }

        if (XGetIMValues(im, XNQueryInputStyle, &styles, NULL)) {
            throw AException("XIM Can't get styles");
        }
    }
    mHandle = XCreateWindow(gDisplay, gScreen->root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual,
                            CWColormap | CWEventMask | CWCursor, &swa);


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
    glXMakeCurrent(gDisplay, mHandle, context.context);

    if (!glewExperimental) {
        ALogger::info((const char*) glGetString(GL_VERSION));
        ALogger::info((const char*) glGetString(GL_VENDOR));
        ALogger::info((const char*) glGetString(GL_RENDERER));
        glewExperimental = true;
        if (glewInit() != GLEW_OK) {
            throw AException("glewInit failed");
        }
    }

#endif
    assert(glGetError() == 0);

    updateDpi();
    Render::instance().setWindow(this);

    {
        // проверим, а дали ли нам вообще биты трафарета
        GLint stencilBits = 0;
        glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
        assert(stencilBits > 0);
    }

#if defined(_WIN32)
    RECT clientRect;
    GetClientRect(mHandle, &clientRect);
    mSize = {clientRect.right - clientRect.left, clientRect.bottom - clientRect.top};
#endif
}

AWindow::~AWindow() {
    getWindowManager().mWindows.remove(shared_from_this());

#if defined(_WIN32)
    wglMakeCurrent(mDC, nullptr);
    ReleaseDC(mHandle, mDC);

    DestroyWindow(mHandle);
    UnregisterClass(mWindowClass.c_str(), mInst);
#elif defined(ANDROID)
    // TODO close
#else
    XDestroyWindow(gDisplay, mHandle);
#endif
}

extern unsigned char stencilDepth;

void AWindow::redraw() {
    {

        // ограничение фпс

        {
            using namespace std::chrono;
            using namespace std::chrono_literals;

            // ограничение 16мс = не более 60 кадров в секунду
            const auto FRAME_DURATION = 16ms;

            static auto lastFrameTime = 0ms;
            auto now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());


            auto delta = now - lastFrameTime;
            if (FRAME_DURATION > delta) {
                std::this_thread::sleep_for(FRAME_DURATION - delta);
            }
            lastFrameTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
        }

        painter p(mHandle);
        GL::State::activeTexture(0);
        GL::State::bindTexture(GL_TEXTURE_2D, 0);
        GL::State::bindVertexArray(0);
        GL::State::useProgram(0);

        Render::instance().setWindow(this);
        glViewport(0, 0, getWidth(), getHeight());

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
#ifndef ANDROID
        glEnable(GL_MULTISAMPLE);
#else
        glClearColor(1.f, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


        // stencil
        glClearStencil(0);
        glClear(GL_STENCIL_BUFFER_BIT);
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0x00);
        stencilDepth = 0;
        glStencilFunc(GL_EQUAL, 0, 0xff);

        doDrawWindow();

#if defined(_WIN32)
        SwapBuffers(p.mHdc);
#elif defined(ANDROID)

#else
        glXSwapBuffers(gDisplay, mHandle);
#endif
    }
    emit redrawn();
}

void AWindow::quit() {
#if defined(_WIN32)
    // родительское окно должно быть активировано до закрытия дочернего.
    if (mParentWindow) {
        EnableWindow(mParentWindow->mHandle, true);
    }
    ShowWindow(mHandle, SW_HIDE);
#else

#endif

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
}

void AWindow::setWindowStyle(WindowStyle ws) {
#if defined(_WIN32)
    if (ws & WS_NO_RESIZE) {
        SetWindowLongPtr(mHandle, GWL_STYLE,
                         GetWindowLong(mHandle, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW | WS_DLGFRAME | WS_THICKFRAME |
                         WS_SYSMENU | WS_CAPTION);
    } else {
        SetWindowLongPtr(mHandle, GWL_STYLE, GetWindowLong(mHandle, GWL_STYLE) & ~WS_DLGFRAME | WS_OVERLAPPEDWINDOW);
    }
    if (ws & WS_SIMPLIFIED_WINDOW) {
        SetWindowLongPtr(mHandle, GWL_STYLE,
                         GetWindowLong(mHandle, GWL_STYLE) & ~WS_THICKFRAME |
                         WS_SYSMENU | WS_CAPTION);
    } else {
        SetWindowLongPtr(mHandle, GWL_STYLE, GetWindowLong(mHandle, GWL_STYLE) | WS_THICKFRAME);
    }

    if (ws & WS_NO_DECORATORS) {
        LONG lExStyle = GetWindowLong(mHandle, GWL_EXSTYLE);
        lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
        SetWindowLong(mHandle, GWL_EXSTYLE, lExStyle);
        SetWindowPos(mHandle, NULL, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
    }
#else
    // TODO
#endif
}

void AWindow::close() {
    onClosed();
    emit closed();
}

void AWindow::updateDpi() {
    emit dpiChanged;
#if defined(_WIN32)
    mDpiRatio = GetDpiForWindow(mHandle) / 96.f;
#else
    mDpiRatio = Platform::getDpiRatio();
#endif
}

void AWindow::minimize() {
#if defined(_WIN32)
    ShowWindow(mHandle, SW_MINIMIZE);
#elif defined(ANDROID)
#else
    XIconifyWindow(gDisplay, mHandle, 0);
#endif
}

glm::ivec2 AWindow::getPos() const {
#if defined(_WIN32)
    RECT r;
    GetWindowRect(mHandle, &r);
    return {r.left, r.top};

#elif defined(ANDROID)
#else
    int x, y;
    Window child;
    XWindowAttributes xwa;
    XTranslateCoordinates(gDisplay, mHandle, gScreen->root, 0, 0, &x, &y, &child);
    XGetWindowAttributes(gDisplay, mHandle, &xwa);

    return {x - xwa.x, y - xwa.y};
#endif
}

TemporaryRenderingContext AWindow::acquireTemporaryRenderingContext() {
    if (painter::painting) {
        return TemporaryRenderingContext(nullptr);
    }

    return TemporaryRenderingContext(_new<painter>(mHandle));
}

void AWindow::setSize(int width, int height) {
    AViewContainer::setSize(width, height);
#if defined(_WIN32)
    auto pos = getPos();

    RECT r = {0, 0, width, height};
    AdjustWindowRectEx(&r, GetWindowLongPtr(mHandle, GWL_STYLE), false, GetWindowLongPtr(mHandle, GWL_EXSTYLE));
    MoveWindow(mHandle, pos.x, pos.y, r.right - r.left, r.bottom - r.top, false);
#elif defined(ANDROID)
#else
    XResizeWindow(gDisplay, mHandle, width, height);
#endif
}

void AWindow::onMouseMove(glm::ivec2 pos) {
    AViewContainer::onMouseMove(pos);

    if (auto v = getViewAtRecusrive(pos)) {
        mCursor = v->getCursor();
    }
}

void AWindow::onFocusAcquired() {
    AViewContainer::onFocusAcquired();
    if (auto v = getFocusedView()) {
        v->onFocusAcquired();
    }
}

void AWindow::onFocusLost() {
    AViewContainer::onFocusLost();
    if (auto v = getFocusedView()) {
        v->onFocusLost();
    }
}

void AWindow::onKeyDown(AInput::Key key) {
    emit keyDown(key);
    if (auto v = getFocusedView())
        v->onKeyDown(key);
}

void AWindow::onKeyRepeat(AInput::Key key) {
    if (auto v = getFocusedView())
        v->onKeyRepeat(key);
}

void AWindow::onKeyUp(AInput::Key key) {
    if (auto v = getFocusedView())
        v->onKeyUp(key);
}

void AWindow::onCharEntered(wchar_t c) {
    if (auto v = getFocusedView()) {
        v->onCharEntered(c);
    }
}

void AWindow::setFocusedView(_<AView> view) {
    if (mFocusedView.lock() == view) {
        return;
    }
    if (auto c = getFocusedView()) {
        c->onFocusLost();
    }
    mFocusedView = view;
    if (view) {
        view->onFocusAcquired();
    }
}

AWindow* AWindow::current() {
    return currentWindowStorage();
}

void AWindow::flagRedraw() {
#if defined(_WIN32)
    InvalidateRect(mHandle, nullptr, true);
#elif defined(ANDROID)
    AAndroid::requestRedraw();
#else
    XEvent e = {0};
    e.type = Expose;
    XSendEvent(gDisplay, mHandle, false, 0, &e);
#endif
}

void AWindow::show() {
    if (!getWindowManager().mWindows.contains(shared_from_this())) {
        getWindowManager().mWindows << shared_from_this();
    }
    try {
        mSelfHolder = shared_from_this();
    } catch (...) {
        mSelfHolder = nullptr;
    }
    redraw();
#if defined(_WIN32)
    UpdateWindow(mHandle);
    ShowWindow(mHandle, SW_SHOWNORMAL);
#else
#endif
    emit shown();
}

AWindowManager& AWindow::getWindowManager() const {
    thread_local AWindowManager ourWindowManager;
    return ourWindowManager;
}

//
// Created by alex2772 on 9/14/20.
//

#include "AWindowManager.h"

AWindowManager::AWindowManager(): mHandle(this) {}

AWindowManager::~AWindowManager() {

}

void AWindowManager::notifyProcessMessages() {
    if (!mWindows.empty()) {
        auto handle = mWindows.back()->mHandle;
#if defined(_WIN32)
        PostMessage(handle, WM_USER, 0, 0);

#elif defined(ANDROID)
#else
        DisplayLock displayLock;
        XEvent e = {0};
        e.xexpose.window = handle;
        e.type = Expose;
        XSendEvent(gDisplay, handle, false, 0, &e);
        XFlush(gDisplay);
#endif
    }
}

void AWindowManager::loop() {

#if defined(_WIN32)
    MSG msg;
    for (mLoopRunning = true; mLoopRunning && !mWindows.empty();) {
        if (GetMessage(&msg, nullptr, 0, 0) == 0) {
            break;
        }
        AThread::current()->processMessages();
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

#elif defined(ANDROID)
    AThread::current()->processMessages();
#else
    XEvent ev;
    for (mLoopRunning = true; mLoopRunning && !mWindows.empty();) {
        XNextEvent(gDisplay, &ev);
        auto window = mWindows.front();
        DisplayLock displayLock;
        switch (ev.type) {
            case KeyPress:
            {
                int count = 0;
                KeySym keysym = 0;
                char buf[0x20];
                Status status = 0;
                count = Xutf8LookupString(window->mIC, (XKeyPressedEvent*)&ev, buf, sizeof(buf), &keysym, &status);

                if (count) {
                    AString s(buf);
                    assert(!s.empty());
                    window->onCharEntered(s[0]);
                }
                break;
            }

            case MappingNotify:
                XRefreshKeyboardMapping(&ev.xmapping);
                break;

            case Expose: {
                glm::ivec2 size = {ev.xexpose.width, ev.xexpose.height};
                if (size.x >= 10 && size.y >= 10 && size != window->getSize())
                    window->AViewContainer::setSize(size.x, size.y);
                window->redraw();
                break;
            }
            case DestroyNotify: {
                window->close();
                break;
            }
            case MotionNotify:
                window->onMouseMove({ev.xmotion.x, ev.xmotion.y});
                break;
            case ButtonPress: {
                window->onMousePressed({ev.xbutton.x, ev.xbutton.y}, (AInput::Key)(AInput::LButton + ev.xbutton.button - 1));
                break;
            }
            case ButtonRelease: {
                window->onMouseReleased({ev.xbutton.x, ev.xbutton.y}, (AInput::Key)(AInput::LButton + ev.xbutton.button - 1));
                break;
            }
        }
        AThread::current()->processMessages();
    }
#endif
}
