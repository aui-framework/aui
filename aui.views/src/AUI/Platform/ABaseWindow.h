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

//
// Created by alex2 on 6/9/2021.
//

#pragma once

#include <AUI/View/AViewContainer.h>
#include "AWindowManager.h"
#include "AOverlappingSurface.h"
#include "ADragNDrop.h"
#include <optional>

namespace testing {
    class UITest;
}


class API_AUI_VIEWS ABaseWindow: public AViewContainer {
    friend class SoftwareRenderer;
    friend class testing::UITest;
    friend struct IRenderingContext::Init;

public:

    ABaseWindow();


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
     * @note
     * When displaying a modal dialog and has blocked the parent window, the application must unblock the parent window
     * before the modal dialog destroyed, otherwise, another window will receive the keyboard focus and be activated.
     */
    virtual void blockUserInput(bool blockUserInput = true);

    virtual ~ABaseWindow();


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
     * @see ABaseWindow::preventClickOnPointerRelease
     */
    [[nodiscard]]
    bool isPreventingClickOnPointerRelease() const noexcept {
        return mPreventClickOnPointerRelease;
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

    void updateDpi();

    float getDpiRatio()
    {
        return mDpiRatio;
    }

    _<AView> getFocusedView() const
    {
        return mFocusedView.lock();
    }

    void setProfiledView(const _<AView>& view) {
        mProfiledView = view;
    }

    void setFocusedView(const _<AView>& view);
    void onPointerPressed(const APointerPressedEvent& event) override;

    void onPointerMove(glm::ivec2 pos) override;

    void closeOverlappingSurfacesOnClick();

    bool isFocused() const {
        return mIsFocused;
    }

    [[nodiscard]]
    const glm::ivec2& getMousePos() const {
        return mMousePos;
    }

    void onKeyDown(AInput::Key key) override;


    virtual void focusNextView();
    virtual void flagRedraw();
    virtual void flagUpdateLayout();

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
     * @code{cpp}
     * auto surfaceContainer = AWindow::current()->createOverlappingSurface({0, 0}, {100, 100});
     *
     * ALayoutInflater::inflate(surfaceContainer, Vertical {
     *     Button { "Item1" },
     *     Button { "Item2" },
     *     Button { "Item3" },
     * });
     *
     * surfaceContainer->pack();
     * @endcode
     *
     * To create overlapping surface below some `view`, use AView::getPositionInWindow:
     * @code{cpp}
     * auto surfaceContainer = AWindow::current()->createOverlappingSurface(view->getPositionInWindow() + view->getSize(), {100, 100});
     * @endcode
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
        if (mOverlappingSurfaces.erase(aui::ptr::fake(surface)) > 0) {
            closeOverlappingSurfaceImpl(surface);
        }
    }

    void onFocusLost() override;
    void render() override;
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
     * @brief On a mobile touchscreen device, shows system virtual keyboard.
     * @details
     * On a desktop device does nothing.
     */
    void requestTouchscreenKeyboard();

    /**
     * @brief Hides virtual keyboard if visible
     */
    void hideTouchscreenKeyboard();

    /**
     * @brief Determines whether views should display hover animations.
     * @return false when any keyboard button is pressed
     */
    bool shouldDisplayHoverAnimations() const;


signals:
    emits<>            dpiChanged;
    emits<glm::ivec2>  mouseMove;
    emits<AInput::Key> keyDown;

protected:
    bool mIsFocused = true;

    /**
     * @see ABaseWindow::preventClickOnPointerRelease
     */
    bool mPreventClickOnPointerRelease = false;

    _unique<IRenderingContext> mRenderingContext;

    static ABaseWindow*& currentWindowStorage();

    /**
     * @see ABaseWindow::createOverlappingSurface
     */
    virtual _<AOverlappingSurface> createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) = 0;
    virtual void closeOverlappingSurfaceImpl(AOverlappingSurface* surface) = 0;

    virtual void createDevtoolsWindow();

    static _unique<AWindowManager>& getWindowManagerImpl();

    virtual float fetchDpiFromSystem() const;

    virtual void requestTouchscreenKeyboardImpl();
    virtual void hideTouchscreenKeyboardImpl();

private:
    _weak<AView> mFocusedView;
    _weak<AView> mProfiledView;
    float mDpiRatio = 1.f;
    bool mIgnoreTouchscreenKeyboardRequests = false; // to avoid flickering


    glm::ivec2 mMousePos;
    ASet<_<AOverlappingSurface>> mOverlappingSurfaces;
};


/**
 * @brief Asserts that the macro invocation has been performed in the UI thread.
 * @ingroup useful_macros
 * @details
 *
 */
#define AUI_ASSERT_UI_THREAD_ONLY() { assert(("this method should be used in ui thread only.", (AWindow::current() ? AThread::current() == AWindow::current()->getThread() : AThread::current() == getThread()))); }

/**
 * @brief Asserts that the macro invocation has not been performed in the UI thread.
 * @ingroup useful_macros
 * @details
 *
 */
#define AUI_ASSERT_WORKER_THREAD_ONLY() { assert(("this method should be used in worker thread only.", AThread::current() != AWindow::current()->getThread())); }

