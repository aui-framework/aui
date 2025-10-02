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

//
// Created by alex2 on 6/9/2021.
//

#pragma once

#include <AUI/View/AViewContainer.h>
#include "AUI/Performance/APerformanceFrame.h"
#include "AUI/Performance/APerformanceSection.h"
#include "AWindowManager.h"
#include "AOverlappingSurface.h"
#include "ADragNDrop.h"
#include "AUI/Util/ATouchScroller.h"
#include "ATouchscreenKeyboardPolicy.h"
#include <chrono>
#include <optional>

namespace testing {
    class UITest;
}

class API_AUI_VIEWS AWindowBase: public AViewContainer {
    friend class SoftwareRenderer;
    friend class IPlatformAbstraction;
    friend class testing::UITest;
    friend struct IRenderingContext::Init;

public:
    using BeforeFrameQueue = AMessageQueue<AFakeMutex, IRenderer&>;

    AWindowBase();

    /**
     * @brief Profiling (debugging) settings for this window.
     * @details
     * Can be controlled with devtools window.
     */
    struct Profiling {
        /**
         * @brief View to highlight.
         */
        AProperty<_weak<AView>> highlightView;

        /**
         * @brief Highlight redraw requests.
         */
        AProperty<bool> highlightRedrawRequests = false;

        /**
         * @brief Visually displays render-to-texture caching by decreasing brightness of pixels that didn't updated in
         * this frame. This effect may help to debug AView::redraw issues.
         */
        AProperty<bool> renderToTextureDecay = false;

        /**
         * @brief When set to true, the next time window's markMinContentSizeInvalid, debugger is invoked. Value is
         * reset to false.
         */
        AProperty<bool> breakpointOnMarkMinContentSizeInvalid = false;

        /**
         * @brief When set to true, all rendered strings will display their baselines.
         */
        AProperty<bool> showBaseline = false;
    };


    /**
     * @brief Enables or disables user input for this window.
     * @param blockUserInput whether block user input or not.
     * @details
     * Visually nothing changes, but when the user tries to interact with the window, nothing happens with the window
     * and on some platforms a system sound played notifying the user that the window is blocked.
     *
     * It is userful when you open a modal window and you want the user to complete the action in the modal window first
     * in order to continue interacting with the parent window.
     *
     * When displaying a modal dialog and has blocked the parent window, the application must unblock the parent window
     * before the modal dialog destroyed, otherwise, another window will receive the keyboard focus and be activated.
     */
    virtual void blockUserInput(bool blockUserInput = true);

    virtual ~AWindowBase();


    /**
     * @brief Prevents click action on upcoming pointer release.
     * @details
     * Also disables hover and pressed animations.
     *
     * Used by AScrollArea when it's scroll is triggered.
     *
     * Calls AView::onClickPrevented() over focus chain.
     */
    void preventClickOnPointerRelease();

    /**
     * @see AWindowBase::preventClickOnPointerRelease
     */
    [[nodiscard]]
    bool isPreventingClickOnPointerRelease() const noexcept {
        return mPreventClickOnPointerRelease.valueOr(false);
    }

    /**
     * @brief Iterates over focus chain, from parent to child.
     */
    template<aui::invocable<const _<AView>&> Callback>
    void iterateOverFocusChain(Callback&& callback) {
        for (auto view = mFocusedView.lock(); view;) {
            callback(view);

            auto container = _cast<AViewContainer>(view);
            if (!container) return;
            view = container->focusChainTarget();
        }
    }

    /**
     * @brief Returns previous frame's rendering duration in millis.
     * @details
     * Returns previous frame's rendering duration in millis, including native rendering preparation and buffer
     * swapping. The value does not include the elapsed time between frames.
     *
     * The value is updated after native buffer swap.
     */
    [[nodiscard]]
    virtual unsigned frameMillis() const noexcept = 0;

    static AWindowManager& getWindowManager() {
        return *getWindowManagerImpl();
    }
    template<typename WindowManager, typename... Args>
    static void setWindowManager(Args&&... args) {
        destroyWindowManager(); // destroys previous window manager so IEventLoop::Handle sets window manager to the
                                // previous one BEFORE the new window manager is set
        getWindowManagerImpl() = std::make_unique<WindowManager>(std::forward<Args>(args)...);
    }
    static void destroyWindowManager() {
        getWindowManagerImpl() = nullptr;
    }

    const _unique<IRenderingContext>& getRenderingContext() const {
        return mRenderingContext;
    }

    [[nodiscard]]
    BeforeFrameQueue& beforeFrameQueue() noexcept {
        return mBeforeFrameQueue;
    }

    void updateDpi();

    /**
     * @brief Returns current dpi ratio
     * @see ScalingParams, scaling params affects dpi ratio
     * @details
     * dpi ratio value is rounded to 0.25
     */
    float getDpiRatio()
    {
        return mDpiRatio;
    }

    _<AView> getFocusedView() const
    {
        return mFocusedView.lock();
    }

    void setFocusedView(const _<AView>& view);
    void updateFocusChain();
    void onPointerPressed(const APointerPressedEvent& event) override;

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;

    void closeOverlappingSurfacesOnClick();

    bool isFocused() const {
        return mIsFocused;
    }

    [[nodiscard]]
    const glm::ivec2& getMousePos() const {
        return mMousePos;
    }

    void onKeyDown(AInput::Key key) override;

#if defined(__clang__)
#pragma clang diagnostic push
#endif
#if defined(__CLION_IDE__) || defined(__CLION_IDE_)
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#endif
    virtual void redraw() {
        AView::redraw();
    }
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    virtual void focusNextView();
    virtual void flagRedraw();

    void makeCurrent() {
        currentWindowStorage() = this;
    }

    /**
     * Creates a surface used for context menus and combo boxes. Closure of the surface is controlled by <code>closeOnClick</code> param but can be
     * closed manually by calling closeOverlappingSurface.
     * @param position position where does the surface should be created. It's not exact the top left corner of the
     *        surface but a hint (i.e. if the surface does not fit)
     * @param size size
     * @param closeOnClick when true, overlapped surface is automatically closed when mouse clicked. It's usable for
     *        dropdown and context menus.
     * @return a new surface.
     * @details
     * ```cpp
     * auto surfaceContainer = AWindow::current()->createOverlappingSurface({0, 0}, {100, 100});
     *
     * ALayoutInflater::inflate(surfaceContainer, Vertical {
     *     Button { Label { "Item1" } },
     *     Button { Label "Item2" } },
     *     Button { Label { "Item3" } },
     * });
     *
     * surfaceContainer->pack();
     * ```
     *
     * To create overlapping surface below some `view`, use AView::getPositionInWindow:
     * ```cpp
     * auto surfaceContainer = AWindow::current()->createOverlappingSurface(view->getPositionInWindow() + view->getSize(), {100, 100});
     * ```
     */
    _<AOverlappingSurface> createOverlappingSurface(const glm::ivec2& position,
                                                    const glm::ivec2& size,
                                                    bool closeOnClick = true) {
        return createOverlappingSurface([&](unsigned attempt) -> AOptional<glm::ivec2> {
            switch (attempt) {
                case 0: return position;
                case 1: return glm::clamp(position, {0, 0}, {getSize() - size});
                default: return std::nullopt;
            }
        }, size, closeOnClick);
    }

    /**
     * Create a surface used for context menus and combo boxes. Closure of the surface is controlled by <code>closeOnClick</code> param but can be
     * closed manually by calling closeOverlappingSurface.
     * @param positionFactory a function that return position where does the surface should be created. If the surface
     *        does not fit, the function is called again with previous arg+1.
     * @param size size
     * @param closeOnClick when true, overlapped surface is automatically closed when mouse clicked. It's usable for
     *        dropdown and context menus.
     * @return a new surface.
     */
    _<AOverlappingSurface> createOverlappingSurface(const std::function<AOptional<glm::ivec2>(unsigned)>& positionFactory,
                                                    const glm::ivec2& size,
                                                    bool closeOnClick = true) {
        glm::ivec2 position = {0, 0};
        auto maxPos = getSize() - size;
        for (unsigned index = 0; ; ++index) {
            auto optionalPosition = positionFactory(index);
            if (optionalPosition) {
                position = *optionalPosition;

                if (position.x >= 0 && position.y >= 0 && glm::all(glm::lessThan(position, maxPos))) {
                    break;
                }
            } else {
                break;
            }
        }

        auto tmp = createOverlappingSurfaceImpl(position, size);
        tmp->mParentWindow = this;
        tmp->mCloseOnClick = closeOnClick;
        mOverlappingSurfaces << tmp;
        return tmp;
    }
    void closeOverlappingSurface(AOverlappingSurface* surface) {
        if (mOverlappingSurfaces.erase(aui::ptr::fake_shared(surface)) > 0) {
            closeOverlappingSurfaceImpl(surface);
        }
    }

    void onFocusLost() override;
    void render(ARenderContext context) override;
    void applyGeometryToChildren() override;
    void onPointerReleased(const APointerReleasedEvent& event) override;

    /**
     * @brief Called when the user holds a drag-n-drop object over the window.
     * @param event event data.
     * @return true, if the application accepts the contents of the event, false otherwise.
     * @details
     * This event handler decides does the application accepts the supplied data. If true, the data supplied to the
     * onDragDrop() method will be the same.
     *
     * onDragLeave() method will be called even if the application rejects the event.
     *
     * It's guaranteed that onDragDrop method will not be called when onDragEnter rejects the same data.
     */
    virtual bool onDragEnter(const ADragNDrop::EnterEvent& event);
    virtual void onDragLeave();
    virtual void onDragDrop(const ADragNDrop::DropEvent& event);

    /**
     * @brief On a mobile touchscreen device, requests system virtual keyboard.
     */
    void requestShowTouchscreenKeyboard();

    /**
     * @brief On a mobile touchscreen device, requests hiding system virtual keyboard.
     */
    void requestHideTouchscreenKeyboard();

    /**
     * @brief Determines whether views should display hover animations.
     * @return false when any keyboard button is pressed
     */
    bool shouldDisplayHoverAnimations() const;

    void onScroll(const AScrollEvent& event) override;

    /**
     * @brief Updates cursor by triggering onPointerMove on the same position (mMousePos).
     */
    virtual void forceUpdateCursor();

    bool onGesture(const glm::ivec2& origin, const AGestureEvent& event) override;

    /**
     * @brief double click will be captured only if time elapsed since the previous click is less than DOUBLECLICK_MAX_DURATION
     */
    static constexpr std::chrono::milliseconds DOUBLECLICK_MAX_DURATION = std::chrono::milliseconds(500);

    /**
     * @return Last captured FPS
     * @details
     * FPS is captured every second
     */
    size_t getFps() {
        return mLastCapturedFps;
    }

    void setTouchscreenKeyboardPolicy(ATouchscreenKeyboardPolicy policy) noexcept {
        mKeyboardPolicy = policy;
    }

    struct ScalingParams {
        /**
         * @brief DPI ratio will be multiplied by this factor
         */
        float scalingFactor = 1.f;
        /**
         * @brief If set, DPI ratio will be adjusted to be small enough for proper displaying layout of given size.
         * @details
         * Size in dp
         */
        AOptional<glm::uvec2> minimalWindowSizeDp = std::nullopt;
    };

    /**
     * @brief Sets scaling params
     * @see ScalingParams
     */
    void setScalingParams(ScalingParams params);

    /**
     * @brief Get profiling settings (mutable).
     */
    aui::lazy<Profiling>& profiling() {
        return mProfiling;
    }

    void markMinContentSizeInvalid() override;

signals:
    emits<>            dpiChanged;
    emits<glm::ivec2>  mouseMove;
    emits<AInput::Key> keyDown;
    emits<>            redrawn;
    emits<>            layoutUpdateComplete;

    /**
     * @brief On touch screen keyboard show.
     */
    emits<> touchscreenKeyboardShown;


    /**
     * @brief On touch screen keyboard hide.
     */
    emits<> touchscreenKeyboardHidden;

#if AUI_PROFILING
    emits<APerformanceSection::Datas> performanceFrameComplete;
#endif

protected:
    bool mIsFocused = true;

    /**
     * @see AWindowBase::preventClickOnPointerRelease
     */
    AOptional<bool> mPreventClickOnPointerRelease;

    /**
     * @brief If true, AWindowBase::forceUpdateCursor takes no action.
     */
    bool mForceUpdateCursorGuard = false;

    bool mPerformDoubleClickOnPointerRelease = false;

    std::chrono::milliseconds mLastButtonPressedTime = std::chrono::milliseconds::zero();
    AOptional<APointerIndex> mLastButtonPressed;
    glm::vec2 mLastPosition = {0, 0};

    _unique<IRenderingContext> mRenderingContext;

    static AWindowBase*& currentWindowStorage();

    /**
     * @see AWindowBase::createOverlappingSurface
     */
    virtual _<AOverlappingSurface> createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) = 0;
    virtual void closeOverlappingSurfaceImpl(AOverlappingSurface* surface) = 0;

    virtual void createDevtoolsWindow();

    static _unique<AWindowManager>& getWindowManagerImpl();

    virtual float fetchDpiFromSystem() const;

    virtual void showTouchscreenKeyboardImpl();
    virtual void hideTouchscreenKeyboardImpl();

    void markPixelDataInvalid(ARect<int> invalidArea) override;

private:
    void processTouchscreenKeyboardRequest();

    _weak<AView> mFocusedView;
    aui::lazy<Profiling> mProfiling = [] { return Profiling{}; };
    float mDpiRatio = 1.f;
    ScalingParams mScalingParams;

    BeforeFrameQueue mBeforeFrameQueue;

    ATouchscreenKeyboardPolicy mKeyboardPolicy = ATouchscreenKeyboardPolicy::SHOWN_IF_NEEDED;

    enum class KeyboardRequest {
        NO_OP,
        SHOW,
        HIDE
    };

    KeyboardRequest mKeyboardRequestedState = KeyboardRequest::NO_OP;

    glm::ivec2 mMousePos = {0, 0};
    ASet<_<AOverlappingSurface>> mOverlappingSurfaces;

    struct Scroll {
        APointerIndex pointer;
        ATouchScroller scroller;
    };

    /**
     * @brief Helper structs to handle touchscreen scroll events.
     */
    ASmallVector<Scroll, 10 /* typical max number of fingers */> mScrolls;

    std::chrono::time_point<std::chrono::high_resolution_clock> mLastTimeFpsCaptured = std::chrono::high_resolution_clock::now();
    size_t mFpsCounter = 0;
    size_t mLastCapturedFps = 0;

#if AUI_SHOW_TOUCHES
    struct ShowTouches {
        glm::vec2 press;
        AVector<glm::vec2> moves;
        AOptional<glm::vec2> release;
    };
    AMap<APointerIndex, ShowTouches> mShowTouches;
#endif
};


/**
 * @brief Asserts that the macro invocation has been performed in the UI thread.
 * @ingroup useful_macros
 * @details
 *
 */
#define AUI_ASSERT_UI_THREAD_ONLY() { AUI_ASSERTX(AWindow::current() == nullptr || AThread::current() == AWindow::current()->getThread(), "this method should be used in ui thread only."); }

/**
 * @brief Asserts that the macro invocation has not been performed in the UI thread.
 * @ingroup useful_macros
 * @details
 *
 */
#define AUI_ASSERT_WORKER_THREAD_ONLY() { AUI_ASSERTX(AWindow::current() == nullptr || AThread::current() != AWindow::current()->getThread(), "this method should be used in worker thread only."); }

