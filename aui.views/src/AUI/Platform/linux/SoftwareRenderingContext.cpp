// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include <X11/X.h>
#include <X11/Xdefs.h>

#include <AUI/Platform/SoftwareRenderingContext.h>
#include "AUI/Software/SoftwareRenderer.h"

SoftwareRenderingContext::SoftwareRenderingContext() {

}

SoftwareRenderingContext::~SoftwareRenderingContext() {
    if (mBitmapBlob && !mXImage) free(mBitmapBlob);
}

void SoftwareRenderingContext::destroyNativeWindow(ABaseWindow &window) {
    CommonRenderingContext::destroyNativeWindow(window);
}

void SoftwareRenderingContext::beginPaint(ABaseWindow &window) {
    CommonRenderingContext::beginPaint(window);
    std::memset(mStencilBlob.data(), 0, mStencilBlob.getSize());
}

void SoftwareRenderingContext::endPaint(ABaseWindow &window) {
    CommonRenderingContext::endPaint(window);
    if (auto nativeWindow = dynamic_cast<AWindow*>(&window)) {
        XPutImage(ourDisplay,
                  nativeWindow->nativeHandle(),
                  mGC.get(),
                  mXImage.get(),
                  0, 0, // source x, y
                  0, 0, // dest   x, y
                  nativeWindow->getWidth(), nativeWindow->getHeight());
        XSync(ourDisplay, false);
    }
}

void SoftwareRenderingContext::beginResize(ABaseWindow &window) {

}

static XVisualInfo* vi = nullptr;

void SoftwareRenderingContext::init(const IRenderingContext::Init &init) {
    CommonRenderingContext::init(init);
    static XSetWindowAttributes swa;
    if (!vi) {
        XVisualInfo viTemplate;
        aui::zero(viTemplate);

        auto screen = DefaultScreen(ourDisplay);

        viTemplate.visualid = DefaultVisual(ourDisplay, screen)->visualid;
        viTemplate.c_class = TrueColor;

        int count;
        vi = XGetVisualInfo(CommonRenderingContext::ourDisplay,
                            VisualIDMask | VisualClassMask,
                            &viTemplate,
                            &count);
        if (!vi) {
            throw AException("unable to pick visual info");
        }
        auto cmap = XCreateColormap(ourDisplay, ourScreen->root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask
                         | PointerMotionMask | StructureNotifyMask | PropertyChangeMask | StructureNotifyMask;

    }
    initX11Window(init, swa, vi);
    mBitmapSize = init.window.getSize();
    XGCValues gcv {
        .graphics_exposures = false,
    };
    mGC = aui::ptr::make_unique_with_deleter(XCreateGC(ourDisplay, init.window.nativeHandle(), GCGraphicsExposures, &gcv),
                                             [](GC c) { XFreeGC(CommonRenderingContext::ourDisplay, c); });
    reallocate();
    if (!mXImage) {
        throw AException("unable to create XImage");
    }
    if (Render::getRenderer() == nullptr) {
        Render::setRenderer(std::make_unique<SoftwareRenderer>());
    }
}

void SoftwareRenderingContext::endResize(ABaseWindow &window) {
    mBitmapSize = window.getSize();
    reallocate();
}

void SoftwareRenderingContext::reallocate() {
    mXImage.reset();
    if (mBitmapBlob) {
        free(mBitmapBlob);
    }
    mBitmapBlob = static_cast<uint8_t*>(malloc(mBitmapSize.x * mBitmapSize.y * 4));

    mStencilBlob.reallocate(mBitmapSize.x * mBitmapSize.y);

    mXImage = aui::ptr::make_unique_with_deleter(XCreateImage(ourDisplay,
                                                              vi->visual,
                                                              vi->depth,
                                                              ZPixmap,
                                                              0,
                                                              reinterpret_cast<char*>(mBitmapBlob),
                                                              mBitmapSize.x, mBitmapSize.y,
                                                              32,
                                                              0), [&] (XImage* image) {
        mBitmapBlob = nullptr; // XLib would deallocate memory for us
        XDestroyImage(image);
    });
}

AImage SoftwareRenderingContext::makeScreenshot() {
    AByteBuffer data;
    size_t s = mBitmapSize.x * mBitmapSize.y * 4;
    data.resize(s);
    std::memcpy(data.data(), mBitmapBlob, s);
    return {std::move(data), mBitmapSize, APixelFormat::ARGB | APixelFormat::BYTE};
}

