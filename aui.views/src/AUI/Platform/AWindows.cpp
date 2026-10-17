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
#include "AUI/UITestState.h"
#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Action/AMenu.h>
#include <AUI/View/ATitlebar.h>
#include <AUI/View/AButton.h>

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Devtools/DevtoolsPanel.h>
#include <AUI/Util/ALayoutInflater.h>
#include <AUI/Platform/OpenGLRenderingContext.h>
#include <AUI/Platform/SoftwareRenderingContext.h>

#ifdef AUI_PLATFORM_LINUX
#include <AUI/Platform/linux/IPlatformAbstraction.h>
#endif

using namespace declarative;

namespace {
/**
 * @return whether the parent of a modal window has to be blocked by AUI.
 */
bool needsManualModalBlock() {
#if AUI_PLATFORM_LINUX
    return !IPlatformAbstraction::current().windowHasNativeModality();
#else
    return true;
#endif
}
}   // namespace

bool AWindow::consumesClick(const glm::ivec2& pos) { return AViewContainer::consumesClick(pos); }

AWindow::AWindow() : AWindow(Init {}) {}

AWindow::AWindow(AString title, AMetric width, AMetric height)
  : AWindow(Init { .title = std::move(title), .width = width, .height = height }) {}

AWindow::AWindow(Init init) : mInit(std::move(init)) {
    mWindowTitle = mInit.title;
    if (std::holds_alternative<Init::DecorationExtended>(mInit.decoration)) {
        addAssName(".window-extended");
    }
    mResizableCache = mInit.resizable;
    windowNativePreInit();
    const auto dpi = fetchDpiFromSystem();
    const auto toPx = [&](AMetric m) {
        return m.getUnit() == AMetric::T_DP ? m.getRawValue() * dpi : m.getRawValue();
    };
    setSize(glm::ivec2(int(toPx(mInit.width)), int(toPx(mInit.height))));
}

AWindow::AWindow(PopupTag, AWindow& parent)
  : mInit(Init {
        .title = "MENU",
        .width = 100_dp,
        .height = 100_dp,
        .parent = Init::Parent { .window = parent },
        .decoration = Init::DecorationBorderless { .shadow = false },
        .resizable = false,
    }),
    mIsPopup(true) {
    mWindowTitle = mInit.title;
    mResizableCache = false;
    windowNativePreInit();
    const auto dpi = fetchDpiFromSystem();
    setSize(glm::ivec2(int(100 * dpi), int(100 * dpi)));
}

void AWindow::doDrawWindow() {
    APerformanceSection s("AWindow::doDrawWindow");
    auto& renderer = mRenderingContext->renderer();
    renderer.setWindow(this);
    render({ .clippingRects = { ARect<int> { .p1 = glm::ivec2(0), .p2 = getSize() } }, .render = renderer });
}

void AWindow::createDevtoolsWindow() {
    class DevtoolsWindow : public AWindow {
    public:
        DevtoolsWindow() : AWindow(Init { .title = "Devtools", .width = 500_dp, .height = 400_dp }) {}

    protected:
        void createDevtoolsWindow() override {
            // you can comment out this to forbid issuing devtools on a devtools window, but who I am to disable such a
            // meme thing?
            AWindow::createDevtoolsWindow();
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
    APerformanceFrame frame([&](APerformanceSection::Datas sections) { emit performanceFrameComplete(sections); });
    APerformanceSection s(
        "AWindow::redraw", std::nullopt, fmt::format("frame {}", [] {
            static uint64_t frameIndex = 0;
            return frameIndex++;
        }()));
#endif

    {
        if (isClosed()) {
            return;
        }

        if (mWantsLayoutUpdate) {
            ensureAssUpdated();
            // laying our contents out in less space than they need produces garbage, so we never do: whatever the
            // window manager makes of the minimum size we announce it (see
            // IPlatformAbstraction::windowAnnounceMinMaxSize), contents that still don't fit overflow the window
            // instead of being squeezed into it.
            auto size = getSize();
            const auto maxSize = getMaxSize();
            if (maxSize.x != -1)
                size.x = glm::min(size.x, maxSize.x);
            if (maxSize.y != -1)
                size.y = glm::min(size.y, maxSize.y);
            size = glm::max(size, getMinimumSize());
            AView::setSize(size);
#if AUI_PLATFORM_WIN || AUI_PLATFORM_MACOS
            // win32 and macos let us resize the native window right here (on macos this also announces the min/max
            // size to NSWindow), keeping it in sync with the layout. on the other platforms this would mean arguing
            // with the window manager from inside a paint, so we don't.
            setSize(size);
#endif
            auto before = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
            layout(getPosition(), getSize());
#if AUI_PLATFORM_MACOS
            // also hides the native title when the titlebar is gone.
            applyTitlebarRect();
#elif AUI_PLATFORM_LINUX
            {
                const auto rectOf = [](const AView& v) {
                    return ARect<int>::fromTopLeftPositionAndSize(v.getPositionInWindow(), v.getSize());
                };
                AOptional<ARect<int>> titlebarRect;
                AOptional<ARect<int>> titleRect;
                if (mTitlebar) {
                    titlebarRect = rectOf(*mTitlebar);
                    if (auto title = mTitlebar->windowTitle(); title && title->getVisibility() == Visibility::VISIBLE) {
                        titleRect = rectOf(*title);
                    }
                }
                if (titlebarRect != mLastTitlebarRect || titleRect != mLastWindowTitleRect) {
                    mLastTitlebarRect = titlebarRect;
                    mLastWindowTitleRect = titleRect;
                    // we are in the middle of painting; toolkits don't like when their widgets are changed under
                    // painting.
                    AUI_UI_THREAD_X[this, alive = _weak<void>(mLifetimeToken), titlebarRect, titleRect] {
                        if (alive.expired() || mClosed) {
                            return;
                        }
                        IPlatformAbstraction::current().windowSetTitlebarRect(*this, titlebarRect, titleRect);
                    };
                }
            }
#endif
            auto after = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
            unsigned millis = mFrameMillis = static_cast<unsigned>((after - before).count());
            if (millis > 20) {
                static auto lastNotification = 0ms;
                if (after - lastNotification > 5min) {
                    lastNotification = after;
                    if (millis > 40) {
                        ALogger::warn("Performance") << "Layout took {}ms! Unacceptably bad performance"_format(millis);
                    } else {
                        ALogger::warn("Performance") << "Layout took {}ms! Bad performance"_format(millis);
                    }
                }
            }
#if AUI_PLATFORM_LINUX
            IPlatformAbstraction::current().windowAnnounceMinMaxSize(*this);
#endif
        }
        applyPendingLayoutRoots();

        auto before = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
        {
            APerformanceSection s("IRenderingContext::beginPaint");
            mRenderingContext->beginPaint(*this);
        }
        AUI_DEFER {
            APerformanceSection s("IRenderingContext::endPaint");
            mRenderingContext->endPaint(*this);
        };

#if AUI_PLATFORM_WIN
        mRedrawFlag = true;
#elif AUI_PLATFORM_MACOS
        mRedrawFlag = false;
#endif

        doDrawWindow();

        // measure frame time
        auto after = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
        unsigned millis = mFrameMillis = static_cast<unsigned>((after - before).count());
        if (millis > 20) {
            static auto lastNotification = 0ms;
            if (after - lastNotification > 5min) {
                lastNotification = after;
                if (millis > 40) {
                    ALogger::warn("Performance")
                        << "Frame render took {}ms! Unacceptably bad performance"_format(millis);
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

_<AWindow> AWindow::wrapViewToWindow(const _<AView>& view, Init init) {
    view->setExpanding();

    auto window = _new<AWindow>(std::move(init));
    window->setContents(Stacked { view });
    return window;
}

void AWindow::show() {
    if (mClosed) {
        ALogger::err("AWindow") << "show() called on a closed window; create another window instead:\n"
                                << AStacktrace::capture(1);
        AUI_ASSERTX(false, "closed window can't be shown again");
        return;
    }
    showImpl();
}

void AWindow::hide() {
    if (mClosed) {
        return;
    }
    hideImpl();
}

void AWindow::setTitle(AString title) {
    if (mWindowTitle == title) {
        return;
    }
    mWindowTitle = std::move(title);
    emit titleChanged(mWindowTitle);
    if (mClosed) {
        return;
    }
    setTitleImpl(mWindowTitle);
}

void AWindow::destroyNativeWindow() {
    if (std::exchange(mNativeDestroyed, true)) {
        return;
    }
#if AUI_PLATFORM_WIN
    if (mHandle) {
        ShowWindow(mHandle, SW_HIDE);
    }
#endif
    if (mRenderingContext) {
        mRenderingContext->destroyNativeWindow(*this);
    }
    mHandle = {};
}

void AWindow::close() {
    if (std::exchange(mClosed, true)) {
        return;
    }
    // keep ourselves alive until the end of this function and a bit longer, so slots connected to closed and the code
    // which called us are able to finish their business.
    auto self = std::move(mSelfHolder);
    mSelfHolder = nullptr;

    emit closed();

    if (mInit.parent && mInit.parent->modal && !mInit.parent->window.isClosed() && needsManualModalBlock()) {
        mInit.parent->window.blockUserInput(false);
    }

    destroyNativeWindow();
#if AUI_PLATFORM_WIN
    setLayout(nullptr);
#endif
    getWindowManager().mWindows.removeIf([this](const auto& p) { return p.get() == this; });

    if (self) {
        AThread::current()->enqueue([self = std::move(self)]() mutable { self = nullptr; });
    }
    getWindowManager().notifyProcessMessages();
}

void AWindow::onFocusAcquired() {
    mIsFocused = true;
    AViewContainer::onFocusAcquired();
}

void AWindow::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    ASurface::onPointerMove(pos, event);
    if (UITestState::isTesting()) {
        return;
    }
    AUI_NULLSAFE(mCursor)->applyNativeCursor(this);
#if AUI_PLATFORM_LINUX
    applyResizeEdgeCursor(pos);
#endif
}

void AWindow::onPointerPressed(const APointerPressedEvent& event) {
    if (!UITestState::isTesting() && !mClosed && captionPressed(event)) {
        return;
    }
    ASurface::onPointerPressed(event);
}

bool AWindow::isCaptionAt(const glm::ivec2& pos) {
    auto v = getViewAtRecursive(pos);
    // the deepest view is usually a part of an interactive widget (i.e., a label inside a button), so we check all
    // views from it up to the titlebar.
    for (AView* cur = v.get(); cur != nullptr && cur != this; cur = cur->getParent()) {
        if (dynamic_cast<AButton*>(cur) || cur->getAssNames().contains(".override-title-dragging")) {
            return false;
        }
        if (dynamic_cast<ATitlebar*>(cur)) {
            return true;
        }
    }
    return false;
}

void AWindow::onFocusLost() {
    mIsFocused = false;
    ASurface::onFocusLost();
    if (AMenu::isOpen()) {
        AMenu::close();
    }
}

void AWindow::onKeyDown(AInput::Key key) {
    ASurface::onKeyDown(key);
    if (mFocusNextViewOnTab && key == AInput::Key::TAB) {
        focusNextView();
    }
}

void AWindow::onKeyRepeat(AInput::Key key) {
    if (auto v = getFocusedView())
        v->onKeyRepeat(key);
}

void AWindow::onCloseButtonClicked() { close(); }

void AWindow::setPosition(glm::ivec2 position) { setGeometry(position.x, position.y, getWidth(), getHeight()); }

void AWindow::setSize(glm::ivec2 size) { setGeometry(getPosition().x, getPosition().y, size.x, size.y); }

glm::ivec2 AWindow::getMinimumSize() {
    static constexpr int MAX_MIN_SIZE = 1000;

    const auto styled = getMinSize();
    // the narrowest our contents can be laid out at, and the height they need at exactly that width.
    const int width = glm::max(styled.x, computeMinMaxAxis().min);
    const int height = glm::max(styled.y, measure(AConstraints::fixedInline(width)).y);
    return glm::min(glm::ivec2(width, height), glm::ivec2(MAX_MIN_SIZE));
}

glm::ivec2 AWindow::mapPositionTo(const glm::ivec2& position, _<AWindow> other) {
    return other->mapPosition(unmapPosition(position));
}

AWindowManager::AWindowManager() : mHandle(this) {
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

AWindowManager::~AWindowManager() {}

void AWindow::windowNativePreInit() {
    const int width = int(mInit.width.getRawValue());
    const int height = int(mInit.height.getRawValue());
    mSize = (glm::max) (glm::ivec2 { width, height }, getMinSize());

    currentWindowStorage() = this;

    getWindowManager().initNativeWindow({ *this, mInit.title, width, height, mIsPopup, parentWindow() });

    moveToCenter();

    if (mInit.parent && mInit.parent->modal && needsManualModalBlock()) {
        mInit.parent->window.blockUserInput(true);
    }

    AUI_UI_THREAD_X[this, alive = _weak<void>(mLifetimeToken)] {
        if (alive.expired()) {
            return;
        }
        emit mSizeChanged(getSize());
    };

#if AUI_PLATFORM_LINUX
    // on linux, we have to manually provide an icon for the system.
    // __aui/icon_512x512.png is provided by aui_app cmake command.
    if (auto _icon = AImage::fromUrl(":__aui/icon_512x512.png")) {
        IPlatformAbstraction::current().windowSetIcon(*this, *_icon);
    }
#endif
}

_<AOverlappingSurface> AWindow::createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) {
    class AOverlappingWindow : public AWindow {
    public:
        AOverlappingWindow(AWindow& parent) : AWindow(PopupTag {}, parent) {
            setCustomStyle({ ass::Padding { 0 } });
        }
    };
    auto window = _new<AOverlappingWindow>(*this);
    auto finalPos = unmapPosition(position);
    window->setGeometry(finalPos.x, finalPos.y, size.x, size.y);
    // show later
    AUI_UI_THREAD { window->show(); };

    class MyOverlappingSurface : public AOverlappingSurface {
    public:
        void setOverlappingSurfacePosition(glm::ivec2 position) override { emit positionSet(position); }

        void setOverlappingSurfaceSize(glm::ivec2 size) override { emit sizeSet(size); }

    signals:
        emits<glm::ivec2> positionSet;
        emits<glm::ivec2> sizeSet;
    };

    auto surface = _new<MyOverlappingSurface>();
    ALayoutInflater::inflate(window, surface);
    connect(surface->positionSet, window, [this, window = window.get()](const glm::ivec2& newPos) {
        window->setPosition(unmapPosition(newPos));
    });
    connect(surface->sizeSet, window, [this, window = window.get()](const glm::ivec2& size) { window->setSize(size); });

    return surface;
}

void AWindow::closeOverlappingSurfaceImpl(_<AOverlappingSurface> surface) {
    if (auto c = dynamic_cast<AWindow*>(surface->getParent())) {
        c->close();
    }
}

void AWindow::forceUpdateCursor() {
    if (mForceUpdateCursorGuard) {
        return;
    }
    ASurface::forceUpdateCursor();
    if (!mCursor) {
        mCursor = ACursor::DEFAULT;
    }
    if (UITestState::isTesting()) {
        return;
    }
    mCursor->applyNativeCursor(this);
}

void AWindowManager::initNativeWindow(const IRenderingContext::Init& init) {
#if AUI_PLATFORM_LINUX
    IPlatformAbstraction::current().windowManagerInitNativeWindow(init);
#else
    for (const auto& graphicsApi : ARenderingContextOptions::get().initializationOrder) {
        try {
            std::visit(
                aui::lambda_overloaded {
                    [](const ARenderingContextOptions::DirectX11&) { throw AException("DirectX is not supported"); },
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
                },
                graphicsApi);
            return;
        } catch (const AException& e) {
            ALogger::warn("AWindowManager") << "Unable to initialize graphics API:" << e;
        }
    }
    throw AException("unable to initialize graphics");
#endif
}

bool AWindow::isClosed() const noexcept { return mSelfHolder == nullptr; }
