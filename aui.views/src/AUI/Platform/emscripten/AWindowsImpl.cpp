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


#include <emscripten/html5.h>
#include <emscripten/emscripten.h>

#include "AUI/GL/gl.h"
#include "AUI/GL/GLDebug.h"
#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/IRenderer.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Util/ARandom.h"
#include "AUI/GL/State.h"
#include "AUI/Thread/AThread.h"
#include "AUI/Thread/AThreadPool.h"
#include "AUI/Common/ATimer.h"
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Platform/OpenGLRenderingContext.h"

#include <chrono>
#include <span>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>

#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>
#include <AUI/View/AAbstractTextField.h>


AWindow::~AWindow() {
    destroyNativeWindow();
}


void AWindow::setResizable(bool resizable) {
    // the window is the surface of the application.
}

bool AWindow::isResizable() const { return false; }

bool AWindow::hasNativeWindowControls() const { return false; }

bool AWindow::hasNativeWindowTitle() const { return false; }

bool AWindow::captionPressed(const APointerPressedEvent&) { return false; }

void AWindow::setTitleImpl(const AString& title) {
#if AUI_PLATFORM_EMSCRIPTEN
    emscripten_set_window_title(title.toStdString().c_str());
#endif
}

void AWindow::restore() {

}

void AWindow::minimize() {
}

bool AWindow::isMinimized() const {
    return false;
}


bool AWindow::isMaximized() const {
    return false;
}

void AWindow::maximize() {

}

glm::ivec2 AWindow::getWindowPosition() const {
    return {0, 0};
}


void AWindow::flagRedraw() {
    mRedrawFlag = true;
}

float AWindow::fetchDpiFromSystem() const {
    return emscripten_get_device_pixel_ratio();
}


void AWindow::setGeometry(int x, int y, int width, int height) {
    // the window is the page: it always takes the whole canvas, whatever size the application asks for
    if (mRenderingContext) {
        const auto canvas = [] {
            glm::ivec2 size;
            emscripten_get_canvas_element_size("#canvas", &size.x, &size.y);
            return size;
        }();
        if (canvas.x > 0 && canvas.y > 0) {
            width = canvas.x;
            height = canvas.y;
        }
    }
    AViewContainer::setPosition({0, 0});
    AUI_NULLSAFE(mRenderingContext)->beginResize(*this);
    AViewContainer::setSize({width, height});
    AUI_NULLSAFE(mRenderingContext)->endResize(*this);
}

glm::ivec2 AWindow::mapPosition(const glm::ivec2& position) {
    return position - getWindowPosition();
}
glm::ivec2 AWindow::unmapPosition(const glm::ivec2& position) {
    return position + getWindowPosition();
}


void AWindow::setIcon(const AImage& image) {
}

void AWindow::hideImpl() {
}

void AWindow::activate(const AString&) {
    show();
}

void AWindowManager::notifyProcessMessages() {
}



namespace {
    // the browser reports the canvas's size in device pixels (ResizeObserver): then it alone resizes the canvas
    bool gCanvasObserved = false;

    void resizeCanvas(ASurface* window, glm::ivec2 size) {
        ALogger::info("Emscripten") << "px size: " << size;
        emscripten_set_canvas_element_size("#canvas", size.x, size.y);
        if (!window) {
            return;
        }
        window->layout(0, 0, size.x, size.y);
        window->updateDpi();   // the device pixel ratio changes with the browser zoom
    }
}

namespace aui::emscripten {
    void applySize(ASurface* window, glm::dvec2 cssSize) {
        if (gCanvasObserved) {
            return;
        }
        // css size * device pixel ratio is a guess at the canvas's device size: the browser snaps the box to device
        // pixels in its own way, and a canvas a pixel off is stretched over the page, blurring everything. Rounded,
        // not truncated: that is what the browser does for the canvas at the page's origin
        resizeCanvas(window, glm::ivec2(glm::round(cssSize * emscripten_get_device_pixel_ratio())));
    }
}

extern "C" EMSCRIPTEN_KEEPALIVE void aui_emscripten_canvas_resized(int width, int height) {
    auto window = dynamic_cast<AWindow*>(ASurface::current());
    resizeCanvas(window, { width, height });
    if (window) {
        // the observer runs after this frame was drawn, and resizing has cleared the canvas: draw it again before the
        // browser shows it
        window->redraw();
    }
}

// true if the browser reports the canvas's device pixel size
EM_JS(bool, aui_emscripten_observe_canvas, (), {
    if (typeof ResizeObserver === 'undefined') {
        return false;
    }
    const observer = new ResizeObserver(function (entries) {
        const entry = entries[entries.length - 1];
        const exact = [entry.devicePixelContentBoxSize[0].inlineSize, entry.devicePixelContentBoxSize[0].blockSize];
        const guess = [entry.contentRect.width * devicePixelRatio, entry.contentRect.height * devicePixelRatio];
        // device emulation (DevTools device mode) reports css pixels here: then the guess is the better size
        const trusted = Math.abs(exact[0] - guess[0]) <= 1 && Math.abs(exact[1] - guess[1]) <= 1;
        _aui_emscripten_canvas_resized.apply(null, trusted ? exact : guess.map(Math.round));
    });
    try {
        observer.observe(Module.canvas || document.getElementById('canvas'), { box: 'device-pixel-content-box' });
        return true;
    } catch (e) {
        return false;   // no device-pixel-content-box (Safari)
    }
});

namespace aui::emscripten {
void setKeyDown(AInput::Key k, bool down);   // AInputImpl.cpp
}

namespace {
    // The canvas covers the page from its top left corner (docs/emscripten.md), so client coordinates
    // are canvas coordinates. Move/up events are listened on the window: a drag that leaves the canvas still ends.
    glm::vec2 toPixels(double clientX, double clientY) {
        return glm::vec2(glm::dvec2{clientX, clientY} * emscripten_get_device_pixel_ratio());
    }

    glm::vec2 position(const EmscriptenMouseEvent* e) { return toPixels(e->clientX, e->clientY); }

    APointerIndex button(const EmscriptenMouseEvent* e) {
        // DOM: 0 = left, 1 = middle, 2 = right; AInput::LBUTTON, CBUTTON, RBUTTON are the same values
        if (e->button < 0 || e->button > AInput::RBUTTON) {
            return APointerIndex::button(AInput::LBUTTON);
        }
        return APointerIndex::button(AInput::Key(e->button));
    }

    bool layout(int eventType, const EmscriptenUiEvent* event, void *userData) {
        aui::emscripten::applySize(ASurface::current(), glm::dvec2{event->windowInnerWidth, event->windowInnerHeight});
        return true;
    }

    bool onMouseMove(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
        AUI_NULLSAFE(ASurface::current())->onPointerMove(position(mouseEvent), { .pointerIndex = button(mouseEvent) });
        return false;
    }

    bool onMousePressed(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
        AUI_NULLSAFE(ASurface::current())->onPointerPressed({
            .position = position(mouseEvent),
            .pointerIndex = button(mouseEvent),
        });
        return true;
    }

    bool onMouseReleased(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
        AUI_NULLSAFE(ASurface::current())->onPointerReleased({
            .position = position(mouseEvent),
            .pointerIndex = button(mouseEvent),
        });
        return false;
    }

    bool onWheel(int eventType, const EmscriptenWheelEvent* e, void* userData) {
        const auto lineHeight = 40.0 * emscripten_get_device_pixel_ratio();
        const double scale = [&] {
            switch (e->deltaMode) {
                case DOM_DELTA_LINE: return lineHeight;
                case DOM_DELTA_PAGE: return lineHeight * 10;
                default: return emscripten_get_device_pixel_ratio();
            }
        }();
        AUI_NULLSAFE(ASurface::current())->onScroll({
            .origin = position(&e->mouse),
            .delta = glm::vec2(glm::dvec2{e->deltaX, e->deltaY} * scale),
        });
        return true;
    }

    /// Touches are fingers: AUI turns them into clicks, drags and kinetic scrolling the same way as on Android.
    /// preventDefault (return true) keeps the browser from scrolling/zooming the page and from emulating mouse events.
    template<typename Handler>
    bool forChangedTouches(const EmscriptenTouchEvent* e, Handler&& handler) {
        auto* surface = ASurface::current();
        if (!surface) {
            return true;
        }
        for (const auto& t : std::span(e->touches, e->numTouches)) {
            if (t.isChanged) {
                handler(*surface, toPixels(t.clientX, t.clientY), APointerIndex::finger(int(t.identifier % 64)));
            }
        }
        return true;
    }

    bool onTouchStart(int, const EmscriptenTouchEvent* e, void*) {
        return forChangedTouches(e, [](ASurface& s, glm::vec2 pos, APointerIndex finger) {
            s.onPointerPressed({ .position = pos, .pointerIndex = finger });
        });
    }

    bool onTouchMove(int, const EmscriptenTouchEvent* e, void*) {
        return forChangedTouches(e, [](ASurface& s, glm::vec2 pos, APointerIndex finger) {
            s.onPointerMove(pos, { .pointerIndex = finger });
        });
    }

    bool onTouchEnd(int, const EmscriptenTouchEvent* e, void*) {
        return forChangedTouches(e, [](ASurface& s, glm::vec2 pos, APointerIndex finger) {
            s.onPointerReleased({ .position = pos, .pointerIndex = finger });
        });
    }

    /// Browser shortcuts (F5, F12, Ctrl+R...) stay with the browser.
    bool isBrowserShortcut(const EmscriptenKeyboardEvent* e, AInput::Key key) {
        return e->ctrlKey || e->metaKey || (key >= AInput::F1 && key <= AInput::F12);
    }

    bool onKeyDown(int, const EmscriptenKeyboardEvent* e, void*) {
        auto* surface = ASurface::current();
        const auto key = AInput::fromNative(int(e->keyCode));
        aui::emscripten::setKeyDown(key, true);
        if (!surface) {
            return false;
        }
        if (key != AInput::UNKNOWN) {
            if (e->repeat) {
                surface->onKeyRepeat(key);
            } else {
                surface->onKeyDown(key);
            }
        }
        // a printable key: `key` is the character it types (UTF-8)
        if (!e->ctrlKey && !e->metaKey) {
            // named keys ("Enter", "Shift") are longer than one character
            if (const auto typed = AStringView(e->key).toUtf32(); typed.size() == 1) {
                surface->onCharEntered(AChar(typed.front()));
            }
        }
        return !isBrowserShortcut(e, key);
    }

    bool onKeyUp(int, const EmscriptenKeyboardEvent* e, void*) {
        const auto key = AInput::fromNative(int(e->keyCode));
        aui::emscripten::setKeyDown(key, false);
        if (key != AInput::UNKNOWN) {
            AUI_NULLSAFE(ASurface::current())->onKeyUp(key);
        }
        return !isBrowserShortcut(e, key);
    }
}

void AWindowManager::loop() {
    AUI_DO_ONCE {
        emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, false, layout);
        gCanvasObserved = aui_emscripten_observe_canvas();
        emscripten_set_mousedown_callback("#canvas", nullptr, false, onMousePressed);
        emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, false, onMouseMove);
        emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, false, onMouseReleased);
        emscripten_set_wheel_callback("#canvas", nullptr, false, onWheel);
        emscripten_set_touchstart_callback("#canvas", nullptr, false, onTouchStart);
        emscripten_set_touchmove_callback("#canvas", nullptr, false, onTouchMove);
        emscripten_set_touchend_callback("#canvas", nullptr, false, onTouchEnd);
        emscripten_set_touchcancel_callback("#canvas", nullptr, false, onTouchEnd);
        emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, false, onKeyDown);
        emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, false, onKeyUp);
    }


    emscripten_set_main_loop_arg([](void* arg) {
        auto self = (AWindowManager*)arg;
        // single-threaded: the timers and the thread pool tasks are run here, between frames
        ATimer::scheduler().iteration(ASchedulerIteration::DONT_BLOCK_INFINITELY | ASchedulerIteration::DONT_BLOCK_TIMED);
        AThreadPool::global().runQueuedTasks();
        AThread::processMessages();
        if (AWindow::isRedrawWillBeEfficient()) {
            for (auto& window: self->mWindows) {
                if (window->mRedrawFlag) {
                    window->mRedrawFlag = false;
                    AWindow::currentWindowStorage() = window.get();
                    self->mWatchdog.runOperation([&] {
                        window->redraw();
                    });
                }
            }
        }
    }, this, 0, true);
}
void AWindow::showImpl() {
    if (!getWindowManager().mWindows.contains(_cast<AWindow>(aui::ptr::shared_from_this(this)))) {
        getWindowManager().mWindows << _cast<AWindow>(aui::ptr::shared_from_this(this));
    }
    try {
        mSelfHolder = _cast<AWindow>(aui::ptr::shared_from_this(this));
    } catch (...) {
        mSelfHolder = nullptr;
    }
    AThread::current() << [&]() {
        redraw();
    };

    emit shown();
}

void AWindow::blockUserInput(bool block) {

}

void AWindow::allowDragNDrop() {

}

void AWindow::showTouchscreenKeyboardImpl() {
}

void AWindow::hideTouchscreenKeyboardImpl() {
}

void AWindow::moveToCenter() {

}

void AWindow::setMobileScreenOrientation(AScreenOrientation screenOrientation) {
}
