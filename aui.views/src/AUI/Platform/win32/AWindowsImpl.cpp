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
#include <AUI/Platform/Platform.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/Platform/AWindowManager.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/ABaseWindow.h>
#include <AUI/Platform/ACustomWindow.h>

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>
#include <AUI/Traits/arrays.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Devtools/DevtoolsPanel.h>
#include <AUI/Util/ALayoutInflater.h>
#include <AUI/Render/OpenGLRenderer.h>

#include <GL/wglew.h>
#include <AUI/Util/Cache.h>
#include <AUI/Util/AError.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>
#include <AUI/Platform/AMessageBox.h>

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
        bool ok = wglMakeCurrent(nullptr, nullptr);
        assert(ok);
        EndPaint(mHandle, &mPaint);
    }
};


thread_local bool painter::painting = false;

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



AWindow::Context::~Context() {
    wglDeleteContext(hrc);
}

bool AWindow::isRenderingContextAcquired() {
    return painter::painting;
}

void AWindow::quit() {
    getWindowManager().mWindows.remove(shared_from_this());

    // parent window should be activated BEFORE child is closed.
    if (mParentWindow) {
        EnableWindow(mParentWindow->mHandle, true);
    }
    ShowWindow(mHandle, SW_HIDE);

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
}

void AWindow::windowNativePreInit(const AString& name, int width, int height, AWindow* parent, WindowStyle ws) {
    mWindowTitle = name;
    mParentWindow = parent;

    currentWindowStorage() = this;

    connect(closed, this, &AWindow::close);

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

#if defined(_DEBUG)
    GL::setupDebug();
#endif
    //assert(glGetError() == 0);

    updateDpi();
    Render::setWindow(this);

    checkForStencilBits();

    RECT clientRect;
    GetClientRect(mHandle, &clientRect);
    mSize = {clientRect.right - clientRect.left, clientRect.bottom - clientRect.top};
    setWindowStyle(ws);

}

AWindow::~AWindow() {
    wglMakeCurrent(mDC, nullptr);
    ReleaseDC(mHandle, mDC);

    DestroyWindow(mHandle);
    UnregisterClass(mWindowClass.c_str(), mInst);
}

extern unsigned char stencilDepth;

using namespace std::chrono;
using namespace std::chrono_literals;


static auto _gLastFrameTime = 0ms;


void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;
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
}


void AWindow::updateDpi() {
    emit dpiChanged;

    typedef UINT(WINAPI *GetDpiForWindow_t)(_In_ HWND);
    static auto GetDpiForWindow = (GetDpiForWindow_t)GetProcAddress(GetModuleHandleA("User32.dll"), "GetDpiForWindow");
    if (GetDpiForWindow) {
        mDpiRatio = GetDpiForWindow(mHandle) / 96.f;
    } else {
        mDpiRatio = Platform::getDpiRatio();
    }

    onDpiChanged();
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
void AWindow::restore() {
    ShowWindow(mHandle, SW_RESTORE);
}

void AWindow::minimize() {
    ShowWindow(mHandle, SW_MINIMIZE);
}

bool AWindow::isMinimized() const {
    return IsIconic(mHandle);
}


bool AWindow::isMaximized() const {
    return IsZoomed(mHandle);
}

void AWindow::maximize() {
    ShowWindow(mHandle, SW_MAXIMIZE);
}

glm::ivec2 AWindow::getWindowPosition() const {
    RECT r;
    GetWindowRect(mHandle, &r);
    return {r.left, r.top};
}

void AWindow::flagRedraw() {
    if (mRedrawFlag) {
        InvalidateRect(mHandle, nullptr, true);
        mRedrawFlag = false;
    }
}


void AWindow::setSize(int width, int height) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, width, height);
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize(width, height);

    RECT r = {0, 0, width, height};
    AdjustWindowRectEx(&r, GetWindowLongPtr(mHandle, GWL_STYLE), false, GetWindowLongPtr(mHandle, GWL_EXSTYLE));
    MoveWindow(mHandle, x, y, r.right - r.left, r.bottom - r.top, false);
}

glm::ivec2 AWindow::mapPosition(const glm::ivec2& position) {
    POINT p = {position.x, position.y};
    ScreenToClient(mHandle, &p);
    return {p.x, p.y};
}
glm::ivec2 AWindow::unmapPosition(const glm::ivec2& position) {
    POINT p = {position.x, position.y};
    ClientToScreen(mHandle, &p);
    return {p.x, p.y};
}

void AWindow::setIcon(const AImage& image) {
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
}

void AWindow::hide() {
    ShowWindow(mHandle, SW_HIDE);
}


void AWindowManager::notifyProcessMessages() {
    if (!mWindows.empty()) {
        auto& lastWindow = mWindows.back();

        // we don't need to notify MS Windows' message queue about new message if message sent from UI thread.
        if (lastWindow->getThread() != AThread::current()) {
            PostMessage(lastWindow->mHandle, WM_USER, 0, 0);
        }
    }
}


void AWindowManager::loop() {
    MSG msg;
    for (mLoopRunning = true; mLoopRunning && !mWindows.empty();) {
        if (GetMessage(&msg, nullptr, 0, 0) == 0) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        AThread::current()->processMessages();
    }
}