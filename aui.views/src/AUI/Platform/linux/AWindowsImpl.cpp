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
#include "Platform.h"
#include "AMessageBox.h"
#include "AWindowManager.h"
#include "ADesktop.h"
#include "ABaseWindow.h"
#include "ACustomWindow.h"

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>
#include <AUI/Traits/arrays.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>

AWindow::Context AWindow::context = {};

#if AUI_PLATFORM_WIN

#include <GL/wglew.h>
#include <AUI/Util/Cache.h>
#include <AUI/Util/AError.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>



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
            // REMIND VINDOVWS ABOUT MY VINDOW I WANT TO REDRAW!!!
            mRedrawFlag = true;
            flagRedraw();
        }

        //bool ok = wglMakeCurrent(GetDC(mHandle), AWindow::context.hrc);
        //assert(ok);
    }


    switch (uMsg) {
/*
        case WM_SIZING:
            if (!isMinimized()) {
                auto rect = reinterpret_cast<LPRECT>(lParam);
                auto w = rect->right - rect->left;
                auto h = rect->bottom - rect->top;
                wglMakeCurrent(mDC, context.hrc);
                emit resized(w, h);
                AViewContainer::setSize(w, h);
            }
            return true;
            */

        case WM_CREATE: // used for ACustomWindow
            return 0;

        case WM_USER:
            return 0;

        case WM_SETFOCUS:
            onFocusAcquired();
            return 0;

        case WM_KILLFOCUS:
            onFocusLost();
            return 0;


        case WM_PAINT: {
            // process thread messages because queue freezes when window is frequently redrawn
            AThread::current()->processMessages();

            if (!painter::painting) {
                redraw();
            }

            return 0;
        }
        case WM_KEYDOWN:
            if (lParam & (1 << 30)) {
                // autoupdate
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
                RECT windowRect, clientRect;
                GetWindowRect(mHandle, &windowRect);
                GetClientRect(mHandle, &clientRect);
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
            if (isMousePressed()) {
                // fix assert(!mPressed);
                onMouseReleased(POS, AInput::LButton);
            }
            onMousePressed(POS, AInput::LButton);
            SetCapture(mHandle);
            return 0;
        case WM_MOUSEWHEEL :
            onMouseWheel(mapPosition(POS), -(GET_WHEEL_DELTA_WPARAM(wParam)));
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

        case WM_DPICHANGED: {
            auto prevDpi = mDpiRatio;
            updateDpi();
            setSize(getWidth() * mDpiRatio / prevDpi, getHeight() * mDpiRatio / prevDpi);
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
            return true;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);

#undef GET_X_LPARAM
#undef GET_Y_LPARAM
#undef POS
}
#elif AUI_PLATFORM_ANDROID

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

#elif AUI_PLATFORM_APPLE
#else

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

#endif

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Devtools/DevtoolsPanel.h>
#include <AUI/Util/ALayoutInflater.h>
#include <AUI/Render/OpenGLRenderer.h>


#if !(AUI_PLATFORM_APPLE)
thread_local bool painter::painting = false;
#endif


AWindow::Context::~Context() {
#if AUI_PLATFORM_WIN
    wglDeleteContext(hrc);
#elif AUI_PLATFORM_ANDROID
#else
    //glXDestroyContext(gDisplay, context);
#endif
}
void AWindow::redraw() {
    if (mUpdateLayoutFlag) {
        mUpdateLayoutFlag = false;
        updateLayout();
    }
#ifdef WIN32
    mRedrawFlag = true;
#endif
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

#if !(AUI_PLATFORM_APPLE)
        painter p(mHandle);
#endif
        GL::State::activeTexture(0);
        GL::State::bindTexture(GL_TEXTURE_2D, 0);
        GL::State::bindVertexArray(0);
        GL::State::useProgram(0);

        Render::setWindow(this);
        glViewport(0, 0, getWidth(), getHeight());

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
#if !(AUI_PLATFORM_ANDROID)
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


#if AUI_PLATFORM_WIN
        SwapBuffers(p.mHdc);
#elif AUI_PLATFORM_ANDROID

        #elif AUI_PLATFORM_APPLE
        // TODO apple
#else
        glXSwapBuffers(gDisplay, mHandle);
#endif
    }
#if AUI_PLATFORM_WIN
    wglMakeCurrent(mDC, context.hrc);
#endif
    emit redrawn();
}
bool AWindow::consumesClick(const glm::ivec2& pos) {
    return AViewContainer::consumesClick(pos);
}

void AWindow::onClosed() {
    quit();
}

void AWindow::doDrawWindow() {
    render();
}

void AWindow::createDevtoolsWindow() {
    class DevtoolsWindow: public AWindow {
    public:
        DevtoolsWindow(): AWindow("Devtools", 500_dp, 400_dp) {}

    protected:
        void createDevtoolsWindow() override {
            // stub
        }
    };
    auto window = _new<DevtoolsWindow>();
    ALayoutInflater::inflate(window, _new<DevtoolsPanel>(this));
    window->show();
}

void AWindow::windowNativePreInit(const AString& name, int width, int height, AWindow* parent, WindowStyle ws) {
    mWindowTitle = name;
    mParentWindow = parent;

    currentWindowStorage() = this;

    connect(closed, this, &AWindow::close);

#if AUI_PLATFORM_WIN
    // CREATE WINDOW
    WNDCLASSEX winClass;

    mInst = GetModuleHandle(nullptr);

    ARandom r;
    for (;;) {
        mWindowClass = "AUI-" + AString::number(r.nextInt());
        winClass.lpszClassName = mWindowClass.c_str();
        winClass.cbSize = sizeof(WNDCLASSEX);
        winClass.style = CS_HREDRAW | CS_VREDRAW;
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

    // used for ACustomWindow
    winProc(mHandle, WM_CREATE, 0, 0);

    if ((ws & WindowStyle::DIALOG) && mParentWindow) {
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

        // context initialization
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

        // vsync
        wglSwapIntervalEXT(true);

        Render::setRenderer(std::make_unique<OpenGLRenderer>());

        //wglMakeCurrent(mDC, nullptr);
    } else {
        bool k = SetPixelFormat(mDC, pxf, &pfd);
        assert(k);
    }

    wglMakeCurrent(mDC, context.hrc);

#elif AUI_PLATFORM_ANDROID
    ALogger::info((const char*) glGetString(GL_VERSION));
    ALogger::info((const char*) glGetString(GL_VENDOR));
    ALogger::info((const char*) glGetString(GL_RENDERER));
    ALogger::info((const char*) glGetString(GL_EXTENSIONS));

#elif AUI_PLATFORM_APPLE
    // TODO apple
#else
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

#endif

#if defined(_DEBUG)
    GL::setupDebug();
#endif
    //assert(glGetError() == 0);

    updateDpi();
    Render::setWindow(this);

    checkForStencilBits();

#if AUI_PLATFORM_WIN
    RECT clientRect;
    GetClientRect(mHandle, &clientRect);
    mSize = {clientRect.right - clientRect.left, clientRect.bottom - clientRect.top};
#endif
    setWindowStyle(ws);

}

AWindow::~AWindow() {
#if AUI_PLATFORM_WIN
    wglMakeCurrent(mDC, nullptr);
    ReleaseDC(mHandle, mDC);

    DestroyWindow(mHandle);
    UnregisterClass(mWindowClass.c_str(), mInst);
#elif AUI_PLATFORM_ANDROID
    // TODO close

#elif AUI_PLATFORM_APPLE
    // TODO apple
#else
    XDestroyWindow(gDisplay, mHandle);
#endif
}

extern unsigned char stencilDepth;

using namespace std::chrono;
using namespace std::chrono_literals;


static auto _gLastFrameTime = 0ms;

_<AWindow> AWindow::wrapViewToWindow(const _<AView>& view, const AString& title, int width, int height, AWindow* parent, WindowStyle ws) {
    view->setExpanding();

    auto window = _new<AWindow>(title, width, height, parent, ws);
    window->setContents(Stacked {
        view
    });
    return window;
}

bool AWindow::isRedrawWillBeEfficient() {
    auto now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    auto delta = now - _gLastFrameTime;
    return 8ms < delta;
}
bool AWindow::isRenderingContextAcquired() {
#if AUI_PLATFORM_APPLE
    return true;
#else
    return painter::painting;
#endif
}
void AWindow::redraw() {
    if (mUpdateLayoutFlag) {
        mUpdateLayoutFlag = false;
        updateLayout();
    }
#ifdef WIN32
    mRedrawFlag = true;
#endif
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

#if !(AUI_PLATFORM_APPLE)
        painter p(mHandle);
#endif
        GL::State::activeTexture(0);
        GL::State::bindTexture(GL_TEXTURE_2D, 0);
        GL::State::bindVertexArray(0);
        GL::State::useProgram(0);

        Render::setWindow(this);
        glViewport(0, 0, getWidth(), getHeight());

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
#if !(AUI_PLATFORM_ANDROID)
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


#if AUI_PLATFORM_WIN
        SwapBuffers(p.mHdc);
#elif AUI_PLATFORM_ANDROID

#elif AUI_PLATFORM_APPLE
        // TODO apple
#else
        glXSwapBuffers(gDisplay, mHandle);
#endif
    }
#if AUI_PLATFORM_WIN
    wglMakeCurrent(mDC, context.hrc);
#endif
    emit redrawn();
}

void AWindow::quit() {
    getWindowManager().mWindows.remove(shared_from_this());
#if AUI_PLATFORM_WIN
    // parent window should be activated BEFORE child is closed.
    if (mParentWindow) {
        EnableWindow(mParentWindow->mHandle, true);
    }
    ShowWindow(mHandle, SW_HIDE);
#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_APPLE
    // TODO apple
#else
    XUnmapWindow(gDisplay, mHandle);
#endif

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
}

void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;
#if AUI_PLATFORM_WIN
    if (!!(ws & WindowStyle::SYS)) {
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
        if (!!(ws & WindowStyle::NO_MINIMIZE_MAXIMIZE)) {
            SetWindowLongPtr(mHandle, GWL_STYLE,
                             GetWindowLong(mHandle, GWL_STYLE) & ~(WS_THICKFRAME |
                                     WS_SYSMENU) | WS_CAPTION);
        } else {
            SetWindowLongPtr(mHandle, GWL_STYLE, GetWindowLong(mHandle, GWL_STYLE) | WS_THICKFRAME);
        }

        if (!!(ws & WindowStyle::NO_RESIZE)) {
            SetWindowLongPtr(mHandle, GWL_STYLE,
                             GetWindowLong(mHandle, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW | WS_DLGFRAME | WS_THICKFRAME |
                                     WS_SYSMENU | WS_CAPTION);
        }
        if (!!(ws & WindowStyle::NO_DECORATORS)) {
            LONG lExStyle = GetWindowLong(mHandle, GWL_EXSTYLE);
            lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
            SetWindowLong(mHandle, GWL_EXSTYLE, lExStyle);
            SetWindowPos(mHandle, NULL, 0, 0, 0, 0,
                         SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }
    }
#elif AUI_PLATFORM_ANDROID

#elif AUI_PLATFORM_APPLE
    // TODO apple
#else
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
#endif
}

void AWindow::close() {
    onClosed();
}

void AWindow::updateDpi() {
    emit dpiChanged;
#if AUI_PLATFORM_WIN
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
    onDpiChanged();
}

void AWindow::restore() {
#if AUI_PLATFORM_WIN
    ShowWindow(mHandle, SW_RESTORE);
#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_APPLE
    // TODO apple
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
#if AUI_PLATFORM_WIN
    ShowWindow(mHandle, SW_MINIMIZE);
#elif AUI_PLATFORM_ANDROID

#elif AUI_PLATFORM_APPLE
    // TODO apple
#else
    XIconifyWindow(gDisplay, mHandle, 0);
#endif
}

bool AWindow::isMinimized() const {
#if AUI_PLATFORM_WIN
    return IsIconic(mHandle);

#elif AUI_PLATFORM_ANDROID
    return false;

#elif AUI_PLATFORM_APPLE
    // TODO apple
    return false;
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
#if AUI_PLATFORM_WIN
    return IsZoomed(mHandle);

#elif AUI_PLATFORM_ANDROID
    return true;
#elif AUI_PLATFORM_APPLE
    // TODO apple
    return false;
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
#if AUI_PLATFORM_WIN
    ShowWindow(mHandle, SW_MAXIMIZE);

#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_APPLE
    // TODO apple
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
#if AUI_PLATFORM_WIN
    RECT r;
    GetWindowRect(mHandle, &r);
    return {r.left, r.top};

#elif AUI_PLATFORM_ANDROID
    return {0, 0};
#elif AUI_PLATFORM_APPLE
    // TODO apple
    return {0, 0};
#else
    int x, y;
    Window child;
    XWindowAttributes xwa;
    XTranslateCoordinates(gDisplay, mHandle, gScreen->root, 0, 0, &x, &y, &child);
    XGetWindowAttributes(gDisplay, mHandle, &xwa);

    return {x, y};
#endif
}

void AWindow::onFocusAcquired() {
    mIsFocused = true;
    AViewContainer::onFocusAcquired();
    /*
    if (auto v = getFocusedView()) {
        v->onFocusAcquired();
    }*/
}

void AWindow::onMouseMove(glm::ivec2 pos) {
    ABaseWindow::onMouseMove(pos);
}

void AWindow::onFocusLost() {
    mIsFocused = false;
    ABaseWindow::onFocusLost();
    if (AMenu::isOpen()) {
        AMenu::close();
    }
    /*
    if (auto v = getFocusedView()) {
        v->onFocusLost();
    }*/
}

void AWindow::onKeyRepeat(AInput::Key key) {
    if (auto v = getFocusedView())
        v->onKeyRepeat(key);
}

ABaseWindow* AWindow::current() {
    return currentWindowStorage();
}

bool AWindow::shouldDisplayHoverAnimations() {
    return current()->isFocused() && !AInput::isKeyDown(AInput::LButton)
                                  && !AInput::isKeyDown(AInput::CButton)
                                  && !AInput::isKeyDown(AInput::RButton);
}

void AWindow::flagRedraw() {
#if AUI_PLATFORM_WIN
    if (mRedrawFlag) {
        InvalidateRect(mHandle, nullptr, true);
        mRedrawFlag = false;
    }
#elif AUI_PLATFORM_ANDROID
    AAndroid::requestRedraw();
#else
    mRedrawFlag = true;
#endif
}

void AWindow::flagUpdateLayout() {
    flagRedraw();
    mUpdateLayoutFlag = true;
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
#if AUI_PLATFORM_WIN
    UpdateWindow(mHandle);
    ShowWindow(mHandle, SW_SHOWNORMAL);
#else
#endif
    emit shown();
}

void AWindow::onCloseButtonClicked() {
    emit closed();
}

void AWindow::setSize(int width, int height) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, width, height);

#if AUI_PLATFORM_WIN
#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_APPLE
    // TODO apple
#else
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
#endif
}

void AWindow::setPosition(const glm::ivec2& position) {
    setGeometry(position.x, position.y, getWidth(), getHeight());
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize(width, height);

#if AUI_PLATFORM_WIN
    RECT r = {0, 0, width, height};
    AdjustWindowRectEx(&r, GetWindowLongPtr(mHandle, GWL_STYLE), false, GetWindowLongPtr(mHandle, GWL_EXSTYLE));
    MoveWindow(mHandle, x, y, r.right - r.left, r.bottom - r.top, false);
#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_APPLE
    // TODO apple
#else
    XMoveWindow(gDisplay, mHandle, x, y);
    XResizeWindow(gDisplay, mHandle, width, height);
#endif
}

glm::ivec2 AWindow::mapPosition(const glm::ivec2& position) {
#if AUI_PLATFORM_WIN
    POINT p = {position.x, position.y};
    ScreenToClient(mHandle, &p);
    return {p.x, p.y};
#else
    return position - getWindowPosition();
#endif
}
glm::ivec2 AWindow::unmapPosition(const glm::ivec2& position) {
#if AUI_PLATFORM_WIN
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
#if AUI_PLATFORM_WIN
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

void AWindow::hide() {
#if AUI_PLATFORM_WIN
    ShowWindow(mHandle, SW_HIDE);

#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_APPLE
    // TODO apple
#else
    XUnmapWindow(gDisplay, mHandle);
#endif
}

_<AView> AWindow::determineSharedPointer() {
    return (shared_from_this());
}


// HELPER FUNCTIONS FOR XLIB

#if AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_LINUX

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


AWindowManager::AWindowManager(): mHandle(this) {}

AWindowManager::~AWindowManager() {

}

void AWindowManager::notifyProcessMessages() {
#if AUI_PLATFORM_ANDROID
    AAndroid::requestRedraw();
#else
    if (!mWindows.empty()) {
#if AUI_PLATFORM_WIN
        auto& lastWindow = mWindows.back();

        // we don't need to notify MS Windows' message queue about new message if message sent from UI thread.
        if (lastWindow->getThread() != AThread::current()) {
            PostMessage(lastWindow->mHandle, WM_USER, 0, 0);
        }
#elif AUI_PLATFORM_APPLE
    // TODO apple
#else
    mXNotifyCV.notify_all();
#endif
    }
#endif
}


_<AOverlappingSurface> AWindow::createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) {
    class AOverlappingWindow: public AWindow {
    public:
        AOverlappingWindow(AWindow* parent):
        AWindow("MENU", 100, 100, parent, WindowStyle::SYS) {
            setCustomAss({ ass::Padding { 0 } });
        }
    };
    auto window = _new<AOverlappingWindow>(this);
    auto finalPos = unmapPosition(position);
    window->setGeometry(finalPos.x, finalPos.y, size.x, size.y);
    // show later
    window->show();

    auto surface = _new<AOverlappingSurface>();
    ALayoutInflater::inflate(window, surface);

    return surface;
}

void AWindow::closeOverlappingSurfaceImpl(AOverlappingSurface* surface) {
    if (auto c = dynamic_cast<AWindow*>(surface->getParent())) {
        c->close();
    }
}

void AWindowManager::loop() {

#if AUI_PLATFORM_WIN
    MSG msg;
    for (mLoopRunning = true; mLoopRunning && !mWindows.empty();) {
        if (GetMessage(&msg, nullptr, 0, 0) == 0) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        AThread::current()->processMessages();
    }

#elif AUI_PLATFORM_ANDROID
    AThread::current()->processMessages();
    if (!mWindows.empty()) {
        mWindows.back()->redraw();
    }
#elif AUI_PLATFORM_APPLE
    // TODO apple
#else
    XEvent ev;
    for (mLoopRunning = true; mLoopRunning && !mWindows.empty();) {
        xProcessEvent(ev);
    }
#endif
}

#if AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_LINUX
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


#endif
