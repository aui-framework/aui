//
// Created by Alex2772 on 12/7/2021.
//

#include <AUI/GL/gl.h>
#include <AUI/Platform/OpenGLWindowInitializer.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/GL/GLDebug.h>

#include <GL/wglew.h>
#include <AUI/GL/OpenGLRenderer.h>


void
OpenGLWindowInitializer::initNativeWindow(AWindow& window, const AString& name, int width, int height, WindowStyle ws,
                                          AWindow* parent) {

    CommonWindowInitializer::initNativeWindow(window, name, width, height, WindowStyle::DEFAULT, parent);



    // INITIALIZE OPENGL
    static PIXELFORMATDESCRIPTOR pfd;
    static int pxf;
    if (mHrc == nullptr) {
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
        } fakeWindow(CreateWindowEx(WS_EX_DLGMODALFRAME, window.mWindowClass.c_str(), name.c_str(), WS_OVERLAPPEDWINDOW,
                                    GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2,
                                    GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2, width, height,
                                    parent != nullptr ? parent->mHandle : nullptr, nullptr, window.mInst, nullptr));

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
        mHrc = wglCreateContext(fakeWindow.mDC);
        wglMakeCurrent(fakeWindow.mDC, mHrc);

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

            wglChoosePixelFormatARB(window.mDC, iPixelFormatAttribList, nullptr, 1, &pxf, &iNumFormats);
            assert(iNumFormats);
            DescribePixelFormat(window.mDC, pxf, sizeof(pfd), &pfd);
            k = SetPixelFormat(window.mDC, pxf, &pfd);
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
            HGLRC CompHRC = wglCreateContextAttribsARB(window.mDC, nullptr, attribs);
            if (CompHRC && wglMakeCurrent(window.mDC, CompHRC)) {
                wglDeleteContext(mHrc);
                mHrc = CompHRC;
            } else
                throw std::runtime_error("Failed to create OpenGL 2.0 context");
        }
        ALogger::info("OpenGL context is ready");

        // vsync
        wglSwapIntervalEXT(true);

        Render::setRenderer(std::make_unique<OpenGLRenderer>());

        //wglMakeCurrent(mDC, nullptr);
    } else {
        bool k = SetPixelFormat(window.mDC, pxf, &pfd);
        assert(k);
    }

    wglMakeCurrent(window.mDC, mHrc);

#if defined(_DEBUG)
    GL::setupDebug();
#endif
    //assert(glGetError() == 0);

    window.updateDpi();
    Render::setWindow(&window);

    window.checkForStencilBits();

    RECT clientRect;
    GetClientRect(window.mHandle, &clientRect);
    window.mSize = {clientRect.right - clientRect.left, clientRect.bottom - clientRect.top};
}

void OpenGLWindowInitializer::destroyNativeWindow(AWindow& window) {
    endPaint(window);
}

void OpenGLWindowInitializer::beginPaint(AWindow& window) {
    bool ok = wglMakeCurrent(window.mHdc, mHrc);
    assert(ok);
}

void OpenGLWindowInitializer::beginResize(AWindow& window) {
    wglMakeCurrent(window.mDC, mHrc);
}

void OpenGLWindowInitializer::endPaint(AWindow& window) {
    SwapBuffers(window.mHdc);
    wglMakeCurrent(nullptr, nullptr);
}

OpenGLWindowInitializer::~OpenGLWindowInitializer() {
    wglDeleteContext(mHrc);
}
