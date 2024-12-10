/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "AUI/Common/AObject.h"
#include "AUI/Common/AString.h"
#include "AUI/Performance/APerformanceFrame.h"
#include "AUI/Performance/APerformanceSection.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Thread/AThread.h"
#include "SoftwareRenderingContext.h"
#include "ARenderingContextOptions.h"
#include "AUI/Traits/callables.h"
#include "AUI/Util/ARaiiHelper.h"
#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Action/AMenu.h>

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Devtools/DevtoolsPanel.h>
#include <AUI/Util/ALayoutInflater.h>
#include <AUI/Platform/OpenGLRenderingContext.h>
#include <AUI/Platform/SoftwareRenderingContext.h>



bool AWindow::consumesClick(const glm::ivec2& pos) {
    return AViewContainer::consumesClick(pos);
}

void AWindow::onClosed() {
    emit closed();
    quit();
}

void AWindow::doDrawWindow() {
    APerformanceSection s("AWindow::doDrawWindow");
    auto& renderer = mRenderingContext->renderer();
    renderer.setWindow(this);
    render({.clippingRects = { ARect<int>{ .p1 = glm::ivec2(0), .p2 = getSize() } }, .render = renderer });
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



using namespace std::chrono;
using namespace std::chrono_literals;


static auto _gLastFrameTime = 0ms;

bool AWindow::isRedrawWillBeEfficient() {
    auto now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    auto delta = now - _gLastFrameTime;
    return 8ms < delta;
}
void AWindow::redraw() {
#if AUI_PROFILING
    APerformanceFrame frame([&](APerformanceSection::Datas sections) {
        emit performanceFrameComplete(sections);
    });
    APerformanceSection s("AWindow::redraw", std::nullopt, fmt::format("frame {}", [] { static uint64_t frameIndex = 0; return frameIndex++; }()));
#endif

    {
        if (isClosed()) {
            return;
        }
        auto before = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
        {
            APerformanceSection s("IRenderingContext::beginPaint");
            mRenderingContext->beginPaint(*this);
        }
        AUI_DEFER {
            APerformanceSection s("IRenderingContext::endPaint");
            mRenderingContext->endPaint(*this);
        };

        if (mMarkedMinContentSizeInvalid) {
            AUI_REPEAT(2) { // AText may trigger extra layout update
                applyGeometryToChildrenIfNecessary();
            }
            mMarkedMinContentSizeInvalid = false;
        }
#if AUI_PLATFORM_WIN
        mRedrawFlag = true;
#elif AUI_PLATFORM_MACOS
        mRedrawFlag = false;
#endif
        doDrawWindow();

        // measure frame time
        auto after = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
        unsigned millis = mFrameMillis = unsigned((after - before).count());
        if (millis > 20) {
            static auto lastNotification = 0ms;
            if (after - lastNotification > 5min) {
                lastNotification = after;
                if (millis > 40) {
                    ALogger::warn("Performance") << "Frame render took {}ms! Unacceptably bad performance"_format(millis);
                } else {
                    ALogger::warn("Performance") << "Frame render took {}ms! Bad performance"_format(millis);
                }
            }
        }
    }
    {
        APerformanceSection s2("emit redrawn");
        emit redrawn();
    }
}

_<AWindow> AWindow::wrapViewToWindow(const _<AView>& view, const AString& title, int width, int height, AWindow* parent, WindowStyle ws) {
    view->setExpanding();

    auto window = _new<AWindow>(title, width, height, parent, ws);
    window->setContents(Stacked {
            view
    });
    return window;
}

void AWindow::close() {
    onClosed();
}


void AWindow::onFocusAcquired() {
    mIsFocused = true;
    AViewContainer::onFocusAcquired();
}

void AWindow::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AWindowBase::onPointerMove(pos, event);
    AUI_NULLSAFE(mCursor)->applyNativeCursor(this);
}

void AWindow::onFocusLost() {
    mIsFocused = false;
    AWindowBase::onFocusLost();
    if (AMenu::isOpen()) {
        AMenu::close();
    }
}

void AWindow::onKeyDown(AInput::Key key) {
    AWindowBase::onKeyDown(key);
    if (mFocusNextViewOnTab && key == AInput::Key::TAB) {
        focusNextView();
    }
}

void AWindow::onKeyRepeat(AInput::Key key) {
    if (auto v = getFocusedView())
        v->onKeyRepeat(key);
}

AWindowBase* AWindow::current() {
    return currentWindowStorage();
}

void AWindow::onCloseButtonClicked() {
    close();
}


void AWindow::setPosition(glm::ivec2 position) {
    setGeometry(position.x, position.y, getWidth(), getHeight());
}


glm::ivec2 AWindow::mapPositionTo(const glm::ivec2& position, _<AWindow> other) {
    return other->mapPosition(unmapPosition(position));
}


AWindowManager::AWindowManager(): mHandle(this) {
    mHangTimer = _new<ATimer>(10s);
    return;
    AObject::connect(mHangTimer->fired, mHangTimer, [&, thread = AThread::current()] {
        if (mWatchdog.isHang()) {
            ALogger::err("ANR") << "UI hang detected:\n" << thread->threadStacktrace();
            std::exit(-1);
        }
    });
#if !AUI_DEBUG
    mHangTimer->start();
#endif
}


AWindowManager::~AWindowManager() {

}



void AWindow::windowNativePreInit(const AString& name, int width, int height, AWindow* parent, WindowStyle ws) {
    mWindowTitle = name;
    mParentWindow = parent;
    mSize = (glm::max)(glm::ivec2{ width, height }, getMinimumSize());

    currentWindowStorage() = this;

    getWindowManager().initNativeWindow({ *this, name, width, height, ws, parent });

    setWindowStyle(ws);

    ui_thread {
        emit sizeChanged(getSize());
    };
}

_<AOverlappingSurface> AWindow::createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) {
    class AOverlappingWindow: public AWindow {
    public:
        AOverlappingWindow(AWindow* parent):
        AWindow("MENU", 100, 100, parent, WindowStyle::SYS) {
            setCustomStyle({ ass::Padding { 0 } });
        }
    };
    auto window = _new<AOverlappingWindow>(this);
    auto finalPos = unmapPosition(position);
    window->setGeometry(finalPos.x, finalPos.y, size.x, size.y);
    // show later
    ui_thread {
        window->show();
    };

    class MyOverlappingSurface: public AOverlappingSurface {
    public:
        void setOverlappingSurfacePosition(glm::ivec2 position) override {
            emit positionSet(position);
        }

        void setOverlappingSurfaceSize(glm::ivec2 size) override {
            emit sizeSet(size);
        }

    signals:
        emits<glm::ivec2> positionSet;
        emits<glm::ivec2> sizeSet;
    };

    auto surface = _new<MyOverlappingSurface>();
    ALayoutInflater::inflate(window, surface);
    connect(surface->positionSet, window, [this, window = window.get()](const glm::ivec2& newPos) {
        window->setPosition(unmapPosition(newPos));
    });
    connect(surface->sizeSet, window, [this, window = window.get()](const glm::ivec2& size) {
        window->setSize(size);
    });

    return surface;
}

void AWindow::closeOverlappingSurfaceImpl(AOverlappingSurface* surface) {
    if (auto c = dynamic_cast<AWindow*>(surface->getParent())) {
        c->close();
    }
}

void AWindow::forceUpdateCursor() {
    if (mForceUpdateCursorGuard) {
        return;
    }
    AWindowBase::forceUpdateCursor();
    if (!mCursor) {
        mCursor = ACursor::DEFAULT;
    }
    mCursor->applyNativeCursor(this);
}

void AWindowManager::initNativeWindow(const IRenderingContext::Init& init) {
    for (const auto& graphicsApi : ARenderingContextOptions::get().initializationOrder) {
        try {
            std::visit(aui::lambda_overloaded{
                    [](const ARenderingContextOptions::DirectX11&) {
                        throw AException("DirectX is not supported");
                    },
                    [&](const ARenderingContextOptions::OpenGL& config) {
                        auto context = std::make_unique<OpenGLRenderingContext>(config);
                        context->init(init);
                        init.setRenderingContext(std::move(context));
                    },
                    [&](const ARenderingContextOptions::Software&) {
                        auto context = std::make_unique<SoftwareRenderingContext>();
                        context->init(init);
                        init.setRenderingContext(std::move(context));
                    },
            }, graphicsApi);
            return;
        } catch (const AException& e) {
            ALogger::warn("AWindowManager") << "Unable to initialize graphics API:" << e;
        }
    }
    throw AException("unable to initialize graphics");
}

bool AWindow::isClosed() const noexcept {
    return mSelfHolder == nullptr;
}
