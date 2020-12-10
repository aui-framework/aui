
#include "AUI/GL/gl.h"
#include "AUI/GL/GLDebug.h"
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
#include "ADesktop.h"

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Image/Drawables.h>
#include <AUI/Util/kAUI.h>

constexpr bool AUI_DISPLAY_BOUNDS = false;
AWindow::Context AWindow::context = {};

#if defined(_WIN32)

#include <GL/wglew.h>
#include <AUI/Util/Cache.h>
#include <AUI/Util/AError.h>

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
    try {
        if (window)
            return window->winProc(hwnd, uMsg, wParam, lParam);
    } catch (const AException& e) {
        AError::handle(e);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT AWindow::winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
#define POS glm::ivec2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))

    assert(mHandle == hwnd);

    static glm::ivec2 lastWindowSize;

    currentWindowStorage() = this;

    if (uMsg != WM_PAINT) {
        if (!mRedrawFlag) {
            // НАПОМНИТЬ ВЕНДЕ, ЧТО Я ОЧЕНЬ ХОЧУ ПЕРЕРИСОВАТЬ СРАНОЕ ОКНО!!!
            mRedrawFlag = true;
            flagRedraw();
        }
    }

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
            if (!painter::painting) {
                redraw();
            }

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

        case WM_MOVING: {
            auto r = (LPRECT)lParam;
            emit moving(glm::ivec2{r->left, r->top});
            return 0;
        }

        case WM_KEYUP:
            onKeyUp(AInput::fromNative(wParam));
            return 0;

        case WM_CHAR: {
            onCharEntered(wParam);
            return 0;
        }

        case WM_SIZE: {
            if (!isMinimized()) {
                wglMakeCurrent(mDC, context.hrc);
                emit resized(LOWORD(lParam), HIWORD(lParam));
                AViewContainer::setSize(LOWORD(lParam), HIWORD(lParam));

                switch (wParam) {
                    case SIZE_MAXIMIZED:
                        emit maximized();
                        break;
                    case SIZE_MINIMIZED:
                        emit minimized();
                        break;
                    case SIZE_RESTORED:
                        emit restored();
                        break;
                }
            }
            return 0;
        }
        case WM_MOUSEMOVE: {
            onMouseMove(POS);

            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hwnd;
            tme.dwFlags = TME_LEAVE;
            TrackMouseEvent(&tme);

            return 0;
        }
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
                        static auto cursor = LoadCursor(nullptr, IDC_HAND);
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
            onCloseButtonClicked();
            return 0;

        case WM_LBUTTONDOWN:
            onMousePressed(POS, AInput::LButton);
            SetCapture(mHandle);
            return 0;
        case WM_MOUSEWHEEL :
            onMouseWheel(mapPosition(POS), GET_WHEEL_DELTA_WPARAM(wParam));
            return 0;
        case WM_LBUTTONUP: {
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

        case WM_DPICHANGED: {
            typedef UINT(WINAPI *GetDpiForWindow_t)(_In_ HWND);
            static auto GetDpiForWindow = (GetDpiForWindow_t)GetProcAddress(GetModuleHandleA("User32.dll"), "GetDpiForWindow");
            float newDpi;
            if (GetDpiForWindow) {
                newDpi = GetDpiForWindow(mHandle) / 96.f;
            } else {
                newDpi = Platform::getDpiRatio();
            }
            setSize(getWidth() * newDpi / mDpiRatio, getHeight() * newDpi / mDpiRatio);
            mDpiRatio = newDpi;
            updateDpi();
            flagRedraw();
            return 0;
        }

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

struct {
    Atom wmProtocols;
    Atom wmDeleteWindow;
    Atom wmHints;
    Atom wmState;
    Atom netWmState;
    Atom netWmStateMaximizedVert;
    Atom netWmStateMaximizedHorz;


    void init() {
        wmProtocols = XInternAtom(gDisplay, "WM_PROTOCOLS", False);
        wmDeleteWindow = XInternAtom(gDisplay, "WM_DELETE_WINDOW", False);
        wmHints = XInternAtom(gDisplay, "_MOTIF_WM_HINTS", true);
        wmState = XInternAtom(gDisplay, "WM_STATE", true);
        netWmState = XInternAtom(gDisplay, "_NET_WM_STATE", false);
        netWmStateMaximizedVert = XInternAtom(gDisplay, "_NET_WM_STATE_MAXIMIZED_VERT", false);
        netWmStateMaximizedHorz = XInternAtom(gDisplay, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
    }

} gAtoms;



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
    Drawables::cleanup();
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

AWindow::AWindow(const AString& name, int width, int height, AWindow* parent, WindowStyle ws) :
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

    SetWindowLongPtr(mHandle, GWLP_USERDATA, reinterpret_cast<long long int>(this));

    if (mParentWindow && ws & WS_DIALOG) {
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
            ALogger::info((const char*) glGetString(GL_EXTENSIONS));
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
                            //WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
                            //WGL_SAMPLES_ARB, i,
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
    ALogger::info((const char*) glGetString(GL_EXTENSIONS));
#else
    do_once {
        if (!XSupportsLocale() || XSetLocaleModifiers("@im=none") == NULL) {
            throw AException("Your X server does not support locales.");
        }
    }
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
                | PointerMotionMask | StructureNotifyMask | PropertyChangeMask;
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
    }

#endif

#if defined(_DEBUG)
    GL::setupDebug();
#endif
    //assert(glGetError() == 0);

    updateDpi();
    Render::inst().setWindow(this);

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
    setWindowStyle(ws);

}

AWindow::~AWindow() {
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

using namespace std::chrono;
using namespace std::chrono_literals;


// ограничение 16мс = не более 60 кадров в секунду
static auto _gLastFrameTime = 0ms;

bool AWindow::isRedrawWillBeEfficient() {
    auto now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    auto delta = now - _gLastFrameTime;
    return 8ms < delta;
}
void AWindow::redraw() {
#ifdef WIN32
    mRedrawFlag = true;
#endif
    {

        // ограничение фпс

        {
            auto now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
            auto delta = now - _gLastFrameTime;
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

        Render::inst().setWindow(this);
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
        glStencilMask(0xff);
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_STENCIL_BUFFER_BIT);
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0x00);
        stencilDepth = 0;
        glStencilFunc(GL_EQUAL, 0, 0xff);

        doDrawWindow();


        if constexpr (AUI_DISPLAY_BOUNDS) {
            auto v = getViewAtRecursive(mapPosition(ADesktop::getMousePosition()));
            if (v == nullptr)
                v = shared_from_this();
            apply(v, {
                RenderHints::PushMatrix m;
                Render::inst().setTransform(glm::translate(glm::mat4(1.f), glm::vec3{getPositionInWindow(), 0.f}));
                Render::inst().setFill(Render::FILL_SOLID);
                glEnable(GL_STENCIL_TEST);
                glStencilMask(0xff);
                glStencilOp(GL_INCR, GL_INCR, GL_INCR);
                glStencilFunc(GL_EQUAL, 0, 0xff);

                // content
                {
                    RenderHints::PushColor c;
                    Render::inst().setColor(0x7cb6c180u);
                    Render::inst().drawRect(getPadding().left, getPadding().top,
                                                getWidth() - getPadding().horizontal(), getHeight() - getPadding().vertical());
                }

                // padding
                {
                    RenderHints::PushColor c;
                    Render::inst().setColor(0xbccf9180u);
                    Render::inst().drawRect(0, 0, getWidth(), getHeight());
                }

                // margin
                {
                    RenderHints::PushColor c;
                    Render::inst().setColor(0xffcca4a0u);
                    Render::inst().drawRect(-getMargin().left, -getMargin().top,
                                                getWidth() + getMargin().horizontal(),
                                                getHeight() + getMargin().vertical());
                }

                glDisable(GL_STENCIL_TEST);
                // Подписи
                {
                    int x = -getMargin().left;
                    int y = getHeight() + getMargin().bottom + 2_dp;

                    FontStyle fs;
                    fs.color = 0xffffffffu;
                    fs.fontRendering = FR_ANTIALIASING;
                    fs.size = 9_pt;
                    auto s = Render::inst().preRendererString(getCssNames().back() + "\n"_as +
                                                              AString::number(getSize().x) + "x"_as + AString::number(getSize().y), fs);

                    {
                        RenderHints::PushColor c;
                        Render::inst().setColor(0x00000070u);
                        Render::inst().drawRect(x, y, s.length + 4_dp, fs.size * 2.5 + 2_dp);
                    }
                    Render::inst().drawString(x + 2_dp, y + 1_dp, s);
                }
            });
        }

#if defined(_WIN32)
        SwapBuffers(p.mHdc);
#elif defined(ANDROID)

#else
        glXSwapBuffers(gDisplay, mHandle);
#endif
    }
#if defined(_WIN32)
    wglMakeCurrent(mDC, context.hrc);
#endif
    emit redrawn();
}

void AWindow::quit() {
    getWindowManager().mWindows.remove(shared_from_this());
#if defined(_WIN32)
    // родительское окно должно быть активировано ДО закрытия дочернего.
    if (mParentWindow) {
        EnableWindow(mParentWindow->mHandle, true);
    }
    ShowWindow(mHandle, SW_HIDE);
#elif (__ANDROID__)
#else
    XUnmapWindow(gDisplay, mHandle);
#endif

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
}

void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;
#if defined(_WIN32)
    if (ws & WS_SYS) {
        SetWindowLongPtr(mHandle, GWL_STYLE, GetWindowLong(mHandle, GWL_STYLE) & ~(WS_CAPTION | WS_THICKFRAME
            | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU) | WS_CHILD);
        {
            LONG lExStyle = GetWindowLong(mHandle, GWL_EXSTYLE);
            lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
            SetWindowLong(mHandle, GWL_EXSTYLE, lExStyle);
        }
        SetWindowPos(mHandle, NULL, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

        // small shadow
        SetClassLong(mHandle, GCL_STYLE, GetClassLong(mHandle, GCL_STYLE) | CS_DROPSHADOW);
    } else {
        if (ws & WS_SIMPLIFIED_WINDOW) {
            SetWindowLongPtr(mHandle, GWL_STYLE,
                             GetWindowLong(mHandle, GWL_STYLE) & ~(WS_THICKFRAME |
                             WS_SYSMENU) | WS_CAPTION);
        } else {
            SetWindowLongPtr(mHandle, GWL_STYLE, GetWindowLong(mHandle, GWL_STYLE) | WS_THICKFRAME);
        }

        if (ws & WS_NO_RESIZE) {
            SetWindowLongPtr(mHandle, GWL_STYLE,
                             GetWindowLong(mHandle, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW | WS_DLGFRAME | WS_THICKFRAME |
                             WS_SYSMENU | WS_CAPTION);
        }
        if (ws & WS_NO_DECORATORS) {
            LONG lExStyle = GetWindowLong(mHandle, GWL_EXSTYLE);
            lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
            SetWindowLong(mHandle, GWL_EXSTYLE, lExStyle);
            SetWindowPos(mHandle, NULL, 0, 0, 0, 0,
                         SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }
    }
#else
    if (ws & (WS_SIMPLIFIED_WINDOW | WS_SYS | WS_NO_DECORATORS)) {
        //note the struct is declared elsewhere, is here just for clarity.
//code is from [http://tonyobryan.com/index.php?article=9][1]
        typedef struct Hints
        {
            unsigned long   flags;
            unsigned long   functions;
            unsigned long   decorations;
            long            inputMode;
            unsigned long   status;
        } Hints;

//code to remove decoration
        Hints hints;
        hints.flags = 2;
        hints.decorations = 0;
        XChangeProperty(gDisplay, mHandle, gAtoms.wmHints, gAtoms.wmHints, 32, PropModeReplace,
                (unsigned char *)&hints, 5);
    }
#endif
}

void AWindow::close() {
    onClosed();
}

void AWindow::updateDpi() {
    emit dpiChanged;
#if defined(_WIN32)
    typedef UINT(WINAPI *GetDpiForWindow_t)(_In_ HWND);
    static auto GetDpiForWindow = (GetDpiForWindow_t)GetProcAddress(GetModuleHandleA("User32.dll"), "GetDpiForWindow");
    if (GetDpiForWindow) {
        mDpiRatio = GetDpiForWindow(mHandle) / 96.f;
    } else {
        mDpiRatio = Platform::getDpiRatio();
    }
#else
    mDpiRatio = Platform::getDpiRatio();
#endif
}

void AWindow::restore() {
#if defined(_WIN32)
    ShowWindow(mHandle, SW_RESTORE);
#else
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

bool AWindow::isMinimized() const {
#ifdef _WIN32
    return IsIconic(mHandle);
#else
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
#endif
}


bool AWindow::isMaximized() const {
#ifdef _WIN32
    return IsZoomed(mHandle);
#else
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
#endif
}

void AWindow::maximize() {
#ifdef _WIN32
    ShowWindow(mHandle, SW_MAXIMIZE);
#else
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
#endif
}

glm::ivec2 AWindow::getWindowPosition() const {
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
/*
TemporaryRenderingContext AWindow::acquireTemporaryRenderingContext() {
    if (painter::painting) {
        return TemporaryRenderingContext(nullptr);
    }

    return TemporaryRenderingContext(_new<painter>(mHandle));
}
*/
void AWindow::onMouseMove(glm::ivec2 pos) {
    AViewContainer::onMouseMove(pos);
    auto v = getViewAtRecursive(pos);
    if (v) {
        mCursor = v->getCursor();
    }
    if (!AWindow::shouldDisplayHoverAnimations()) {
        if (auto focused = mFocusedView.lock()) {
            if (focused != v) {
                focused->onMouseMove(pos - focused->getPositionInWindow());
            }
        }
    }

    if constexpr (AUI_DISPLAY_BOUNDS) {
        flagRedraw();
    }
}

void AWindow::onFocusAcquired() {
    mIsFocused = true;
    AViewContainer::onFocusAcquired();
    if (auto v = getFocusedView()) {
        v->onFocusAcquired();
    }
}

void AWindow::onFocusLost() {
    mIsFocused = false;
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
    if (auto c = mFocusedView.lock()) {
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

bool AWindow::shouldDisplayHoverAnimations() {
    return current()->isFocused() && !AInput::isKeyDown(AInput::LButton)
                                  && !AInput::isKeyDown(AInput::CButton)
                                  && !AInput::isKeyDown(AInput::RButton);
}

void AWindow::flagRedraw() {
#if defined(_WIN32)
    if (mRedrawFlag) {
        InvalidateRect(mHandle, nullptr, true);
        mRedrawFlag = false;
    }
#elif defined(ANDROID)
    AAndroid::requestRedraw();
#else
    mRedrawFlag = true;
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
    AThread::current() << [&]() {
        redraw();
    };
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

void AWindow::onCloseButtonClicked() {
    emit closed();
}

void AWindow::setSize(int width, int height) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, width, height);
}

void AWindow::setPosition(const glm::ivec2& position) {
    setGeometry(position.x, position.y, getWidth(), getHeight());
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize(width, height);

#if defined(_WIN32)
    RECT r = {0, 0, width, height};
    AdjustWindowRectEx(&r, GetWindowLongPtr(mHandle, GWL_STYLE), false, GetWindowLongPtr(mHandle, GWL_EXSTYLE));
    MoveWindow(mHandle, x, y, r.right - r.left, r.bottom - r.top, false);
#elif defined(ANDROID)
#else
    XResizeWindow(gDisplay, mHandle, width, height);
#endif
}

glm::ivec2 AWindow::mapPosition(const glm::ivec2& position) {
#ifdef _WIN32
    POINT p = {position.x, position.y};
    ScreenToClient(mHandle, &p);
    return {p.x, p.y};
#else
    return position - getWindowPosition();
#endif
}
glm::ivec2 AWindow::unmapPosition(const glm::ivec2& position) {
#ifdef _WIN32
    POINT p = {position.x, position.y};
    ClientToScreen(mHandle, &p);
    return {p.x, p.y};
#else
    return position + getWindowPosition();
#endif
}

glm::ivec2 AWindow::mapPositionTo(const glm::ivec2& position, _<AWindow> other) {
    return other->mapPosition(unmapPosition(position));
}

void AWindow::setIcon(const AImage& image) {
#ifdef _WIN32
    assert(image.getFormat() & AImage::BYTE);

    if (mIcon) {
        DestroyIcon(mIcon);
    }

    HDC hdcScreen = GetDC(nullptr);

    HDC hdcMemColor = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmpColor = CreateCompatibleBitmap(hdcScreen, image.getWidth(), image.getHeight());
    auto hbmpOldColor = (HBITMAP)SelectObject(hdcMemColor, hbmpColor);

    HDC hdcMemMask = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmpMask = CreateCompatibleBitmap(hdcMemMask, image.getWidth(), image.getHeight());
    auto hbmpOldMask = (HBITMAP)SelectObject(hdcMemMask, hbmpMask);

    for (int y = 0; y < image.getHeight(); ++y) {
        for (int x = 0; x < image.getWidth(); ++x) {
            const uint8_t* color = &image.at(x, y);
            if (image.getFormat() & AImage::RGBA) {
                uint8_t a = 0xffu - color[3];
                SetPixel(hdcMemMask, x, y, RGB(a, a, a));
            }
            SetPixel(hdcMemColor, x, y, RGB(color[0], color[1], color[2]));
        }
    }

    SelectObject(hdcMemColor, hbmpOldColor);
    DeleteObject(hdcMemColor);
    SelectObject(hdcMemMask, hbmpOldMask);
    DeleteObject(hdcMemMask);

    ICONINFO ii;
    ii.fIcon = TRUE;
    ii.hbmMask = hbmpMask;
    ii.hbmColor = hbmpColor;
    mIcon = CreateIconIndirect(&ii);
    DeleteObject(hbmpColor);

    // Clean-up.
    SelectObject(hdcMemColor, hbmpOldColor);
    DeleteObject(hbmpColor);
    DeleteDC(hdcMemColor);
    ReleaseDC(NULL, hdcScreen);

    SendMessage(mHandle, WM_SETICON, ICON_BIG, (LPARAM)mIcon);
#endif
}

// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ для XLIB

#ifdef __linux

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

#endif


//
// Created by alex2772 on 9/14/20.
//

#include "AWindowManager.h"
#include "ACustomWindow.h"

AWindowManager::AWindowManager(): mHandle(this) {}

AWindowManager::~AWindowManager() {

}

void AWindowManager::notifyProcessMessages() {
#if defined(ANDROID)
    AAndroid::requestRedraw();
#else
    if (!mWindows.empty()) {
#if defined(_WIN32)
        auto handle = mWindows.back()->mHandle;
        PostMessage(handle, WM_USER, 0, 0);
#else
    mXNotifyCV.notify_all();
#endif
    }
#endif
}

void AWindowManager::loop() {

#if defined(_WIN32)
    MSG msg;
    for (mLoopRunning = true; mLoopRunning && !mWindows.empty();) {
        if (GetMessage(&msg, nullptr, 0, 0) == 0) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        AThread::current()->processMessages();
    }

#elif defined(ANDROID)
    AThread::current()->processMessages();
    if (!mWindows.empty()) {
        mWindows.back()->redraw();
    }
#else
    XEvent ev;
    for (mLoopRunning = true; mLoopRunning && !mWindows.empty();) {
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
                        if (ev.xclient.message_type == gAtoms.wmProtocols &&
                            ev.xclient.data.l[0] == gAtoms.wmDeleteWindow) {
                            // клик по кнопке закрытия окна
                            auto window = locateWindow(ev.xclient.window);
                            window->onCloseButtonClicked();
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

                        if (count) {
                            AString s(buf);
                            assert(!s.empty());
                            window->onCharEntered(s[0]);
                        }
                        window->onKeyDown(AInput::fromNative(ev.xkey.keycode));
                        break;
                    }
                    case KeyRelease:
                        window = locateWindow(ev.xkey.window);
                        window->onKeyUp(AInput::fromNative(ev.xkey.keycode));
                        break;

                    case ConfigureNotify:
                        if (auto w = _cast<ACustomWindow>(window = locateWindow(ev.xconfigure.window))) {
                            w->handleXConfigureNotify();
                        }

                    case MappingNotify:
                        XRefreshKeyboardMapping(&ev.xmapping);
                        break;

                    case Expose: {
                        window = locateWindow(ev.xexpose.window);
                        glm::ivec2 size = {ev.xexpose.width, ev.xexpose.height};
                        if (size.x >= 10 && size.y >= 10 && size != window->getSize())
                            window->AViewContainer::setSize(size.x, size.y);
                        window->mRedrawFlag = true;
                        break;
                    }
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
                                window->onMouseWheel({ev.xbutton.x, ev.xbutton.y}, -20_dp);
                                break;
                            case 5: // wheel up
                                window->onMouseWheel({ev.xbutton.x, ev.xbutton.y}, 20_dp);
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
                                window let (AWindow, {
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
                }
            }


            std::unique_lock lock(mXNotifyLock);
            mXNotifyCV.wait_for(lock, std::chrono::microseconds(500));
            AThread::current()->processMessages();
            for (auto& window : mWindows) {
                if (window->mRedrawFlag) {
                    window->mRedrawFlag = false;
                    window->redraw();
                }
            }
        } catch(NotFound e) {

        }
    }
#endif
}
