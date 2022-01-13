//
// Created by Alex2772 on 12/9/2021.
//

#include <AUI/Platform/CommonRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Util/AError.h>
#include <X11/extensions/sync.h>
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Util/kAUI.h"

Display* CommonRenderingContext::ourDisplay = nullptr;
Screen* CommonRenderingContext::ourScreen = nullptr;
CommonRenderingContext::Atoms CommonRenderingContext::ourAtoms;


int xerrorhandler(Display* dsp, XErrorEvent* error) {
    if (CommonRenderingContext::ourDisplay == dsp) {
        char errorstring[0x100];
        XGetErrorText(dsp, error->error_code, errorstring, sizeof(errorstring));
        printf("X Error: %s\n", errorstring);
    }
    return 0;
}

void CommonRenderingContext::ensureXLibInitialized() {
    struct DisplayInstance {

    public:
        DisplayInstance() {
            auto d = CommonRenderingContext::ourDisplay = XOpenDisplay(nullptr);
            if (d == nullptr) return;
            XSetErrorHandler(xerrorhandler);
            CommonRenderingContext::ourScreen = DefaultScreenOfDisplay(CommonRenderingContext::ourDisplay);

            CommonRenderingContext::ourAtoms.wmProtocols = XInternAtom(d, "WM_PROTOCOLS", False);
            CommonRenderingContext::ourAtoms.wmDeleteWindow = XInternAtom(d, "WM_DELETE_WINDOW", False);
            CommonRenderingContext::ourAtoms.wmHints = XInternAtom(d, "_MOTIF_WM_HINTS", true);
            CommonRenderingContext::ourAtoms.wmState = XInternAtom(d, "WM_STATE", true);
            CommonRenderingContext::ourAtoms.netWmState = XInternAtom(d, "_NET_WM_STATE", false);
            CommonRenderingContext::ourAtoms.netWmStateMaximizedVert = XInternAtom(d, "_NET_WM_STATE_MAXIMIZED_VERT", false);
            CommonRenderingContext::ourAtoms.netWmStateMaximizedHorz = XInternAtom(d, "_NET_WM_STATE_MAXIMIZED_HORZ", false);
            CommonRenderingContext::ourAtoms.clipboard = XInternAtom(d, "CLIPBOARD", False);
            CommonRenderingContext::ourAtoms.utf8String = XInternAtom(d, "UTF8_STRING", False);
            CommonRenderingContext::ourAtoms.textPlain = XInternAtom(d, "text/plain", False);
            CommonRenderingContext::ourAtoms.textPlainUtf8 = XInternAtom(d, "text/plain;charset=utf-8", False);
            CommonRenderingContext::ourAtoms.auiClipboard = XInternAtom(d, "AUI_CLIPBOARD", False);
            CommonRenderingContext::ourAtoms.incr = XInternAtom(d, "INCR", False);
            CommonRenderingContext::ourAtoms.targets = XInternAtom(d, "TARGETS", False);
            CommonRenderingContext::ourAtoms.netWmSyncRequest = XInternAtom(d, "_NET_WM_SYNC_REQUEST", False);
            CommonRenderingContext::ourAtoms.netWmSyncRequestCounter = XInternAtom(d, "_NET_WM_SYNC_REQUEST_COUNTER", False);
        }

        ~DisplayInstance() {
            //XCloseDisplay(ourDisplay);
            //XFree(ourScreen);

        }
    };
    static DisplayInstance display;
}

void CommonRenderingContext::initX11Window(const IRenderingContext::Init &init, XSetWindowAttributes& swa, XVisualInfo* vi) {
    do_once {
        if (!XSupportsLocale() || XSetLocaleModifiers("@im=none") == NULL) {
            throw AException("Your X server does not support locales.");
        }
    }
    auto& window = init.window;
    ensureXLibInitialized();

    static XIM im;
    static XIMStyles *styles;

    im = XOpenIM(ourDisplay, NULL, NULL, NULL);
    if (im == NULL) {
        throw AException("Could not open input method");
    }

    if (XGetIMValues(im, XNQueryInputStyle, &styles, NULL)) {
        throw AException("XIM Can't get styles");
    }

    window.mHandle = XCreateWindow(ourDisplay,
                                   ourScreen->root,
                                   0, 0,
                                   init.width, init.height,
                                   0,
                                   vi->depth,
                                   InputOutput,
                                   vi->visual,
                                   CWColormap | CWEventMask | CWCursor, &swa);

    // XSync
    {
        XSyncValue value;
        XSyncIntToValue(&value, 0);
        window.mXsyncRequestCounter.counter = XSyncCreateCounter(ourDisplay, value);
        XChangeProperty(ourDisplay,
                        window.mHandle,
                        ourAtoms.netWmSyncRequestCounter,
                        XA_CARDINAL,
                        32,
                        PropModeReplace,
                        (const unsigned char*)&window.mXsyncRequestCounter.counter, 1);

    }

    window.mIC = XCreateIC(im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window.mHandle, NULL);
    if (window.mIC == NULL) {
        throw AException("Could not get IC");
    }
    XSetICFocus(window.mIC);


    XMapWindow(ourDisplay, window.mHandle);

    auto title = init.name.toStdString();
    XStoreName(ourDisplay, window.mHandle, title.c_str());
    XChangeProperty(ourDisplay, window.mHandle, XInternAtom(ourDisplay, "_NET_WM_NAME", false),
                    XInternAtom(ourDisplay, "UTF8_STRING", false), 8, PropModeReplace,
                    reinterpret_cast<const unsigned char*>(title.c_str()), title.length());

    XSetWMProtocols(ourDisplay, window.mHandle, &ourAtoms.wmDeleteWindow, 1);

}

void CommonRenderingContext::init(const Init& init) {


    IRenderingContext::init(init);
}

void CommonRenderingContext::destroyNativeWindow(AWindow& window) {

    XDestroyWindow(ourDisplay, window.mHandle);
}

void CommonRenderingContext::beginPaint(AWindow& window) {
}

void CommonRenderingContext::endPaint(AWindow& window) {
}
