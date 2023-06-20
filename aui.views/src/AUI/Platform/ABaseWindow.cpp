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

#include <AUI/Traits/strings.h>
#include "AUI/Util/ARandom.h"
#include "AUI/Platform/AWindow.h"
#include "ABaseWindow.h"
#include <AUI/Action/AMenu.h>
#include <AUI/Traits/memory.h>
#include <AUI/Util/kAUI.h>
#include <chrono>
#include "APlatform.h"
#include <AUI/Devtools/DevtoolsPanel.h>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Util/ALayoutInflater.h>
#include <AUI/Util/AViewProfiler.h>
#include <AUI/UITestState.h>

ABaseWindow::ABaseWindow() {
    mDpiRatio = APlatform::getDpiRatio();
}

ABaseWindow::~ABaseWindow() {
    if (currentWindowStorage() == this) {
        currentWindowStorage() = nullptr;
    }
}

float ABaseWindow::fetchDpiFromSystem() const {
    return APlatform::getDpiRatio();
}

void ABaseWindow::updateDpi() {
    emit dpiChanged;
    mDpiRatio = UITestState::isTesting() ? 1.f : fetchDpiFromSystem();
    onDpiChanged();
}

_unique<AWindowManager>& ABaseWindow::getWindowManagerImpl() {
    thread_local _unique<AWindowManager> ourWindowManager = std::make_unique<AWindowManager>();
    return ourWindowManager;
}

void ABaseWindow::setFocusedView(const _<AView>& view) {
    if (mFocusedView.lock() == view) {
        return;
    }
    if (auto c = mFocusedView.lock()) {
        c->onFocusLost();
    }

    mFocusedView = view;

    if (view) {
        if (!view->hasFocus()) {
            view->onFocusAcquired();
        }
    }
}

void ABaseWindow::focusNextView() {
    if (!mFocusNextViewOnTab)
        return;

    AView* beginPoint = getFocusedView().get();

    bool triedToSearchFromBeginning = false;

    if (beginPoint == nullptr) {
        beginPoint = this;
        triedToSearchFromBeginning = true;
    }
    auto target = beginPoint;
    while (target != nullptr) {
        if (auto asContainer = dynamic_cast<AViewContainer*>(target)) {
            // container
            if (!asContainer->getViews().empty()) {
                target = asContainer->getViews().first().get();
                continue;
            }
        }
        if (target == beginPoint || !target->handlesNonMouseNavigation() || target->getVisibilityRecursive() == Visibility::GONE) {
            // we should jump to the next element
            if (target->getParent()) {
                // up though hierarchy
                while (auto parent = target->getParent()) {
                    auto& parentViews = parent->getViews();

                    auto index = [&]() -> size_t {
                        for (size_t i = 0; i < parentViews.size(); ++i)
                        {
                            if (parentViews[i].get() == target)
                                return i;
                        }

                        return static_cast<size_t>(-1);
                    }()  + 1;
                    if (index >= parentViews.size()) {

                        // jump to the next container since we already visited all the elements in current container
                        target = target->getParent();
                        if (target == nullptr) {
                            if (triedToSearchFromBeginning) {
                                break;
                            } else {
                                beginPoint = target = this;
                                triedToSearchFromBeginning = true;
                                break;
                            }
                        }
                    } else {
                        target = parentViews[index].get();
                        break;
                    }
                }
            } else {
                // root element
                if (triedToSearchFromBeginning) {
                    // already tried searching from beginning, breaking loop
                    target = nullptr;
                    break;
                } else {
                    // try to search from beginning
                    beginPoint = target = this;
                    triedToSearchFromBeginning = true;
                }
            }
        } else {
            // found something what is not beginPoint
            break;
        }
    }

    if (target != this) {
        if (mFocusedView.lock().get() == target) {
            return;
        }
        if (auto c = mFocusedView.lock()) {
            c->onFocusLost();
        }
        mFocusedView = target->sharedPtr();
        if (target) {
            if (!target->hasFocus()) {
                target->onFocusAcquired();
            }
        }
    }
}

void ABaseWindow::closeOverlappingSurfacesOnClick() {
    // creating copy because of comodification
    AVector<AOverlappingSurface*> surfacesToClose;
    surfacesToClose.reserve(mOverlappingSurfaces.size());
    for (auto& surface : mOverlappingSurfaces) {
        if (surface->isCloseOnClick()) {
            surfacesToClose << surface.get();
        }
    }
    for (auto& surface : surfacesToClose) {
        closeOverlappingSurface(surface);
    }
}

void ABaseWindow::onPointerPressed(const APointerPressedEvent& event) {
    closeOverlappingSurfacesOnClick();
    auto focusCopy = mFocusedView.lock();
    mIgnoreTouchscreenKeyboardRequests = false;
    AViewContainer::onPointerPressed(event);

    if (mFocusedView.lock() != focusCopy && focusCopy != nullptr) {
        if (focusCopy->hasFocus()) {
            focusCopy->onFocusLost();
        }
    }

    hideTouchscreenKeyboard(); // would do hide only if show is not requested on this particular frame
    mIgnoreTouchscreenKeyboardRequests = false;

    // check for double clicks
    using namespace std::chrono;
    using namespace std::chrono_literals;
    static milliseconds lastButtonPressedTime = 0ms;
    static AInput::Key lastButtonPressed = AInput::UNKNOWN;
    static glm::ivec2 lastPosition = {0, 0};

    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    auto delta = now - lastButtonPressedTime;
    if (delta < 500ms && lastPosition == event.position) {
        if (lastButtonPressed == event.button) {
            onPointerDoubleClicked(event);

            lastButtonPressedTime = 0ms;
        }
    } else {
        lastButtonPressedTime = now;
        lastButtonPressed = event.button;
        lastPosition = event.position;
    }
    AMenu::close();
}

void ABaseWindow::onPointerReleased(const APointerReleasedEvent& event) {
    APointerReleasedEvent copy = event;
    copy.triggerClick = !mPreventClickOnPointerRelease;
    mPreventClickOnPointerRelease = false;
    AViewContainer::onPointerReleased(copy);

    // AView::onPointerMove handles cursor shape; need extra call in order to flush
    AViewContainer::onPointerMove(event.position);
}

void ABaseWindow::onPointerMove(glm::ivec2 pos) {
    mMousePos = pos;
    mCursor = ACursor::DEFAULT;
    AViewContainer::onPointerMove(pos);

    emit mouseMove(pos);
}

void ABaseWindow::onKeyDown(AInput::Key key) {
    AViewContainer::onKeyDown(key);
    emit keyDown(key);

#if AUI_DEBUG
    if (key == AInput::F12 && AInput::isKeyDown(AInput::LCONTROL)) {
        createDevtoolsWindow();
    }
#endif
}

void ABaseWindow::createDevtoolsWindow() {
    auto surface = createOverlappingSurface({0, 0}, { 500_dp, 400_dp });
    surface->setCloseOnClick(false);
    ALayoutInflater::inflate(surface, _new<DevtoolsPanel>(this));
}

void ABaseWindow::flagRedraw() {

}

void ABaseWindow::flagUpdateLayout() {

}

void ABaseWindow::render() {
    AViewContainer::render();
    mIgnoreTouchscreenKeyboardRequests = false;

    if (auto v = mProfiledView.lock()) {
        AViewProfiler::displayBoundsOn(*v);
    }
}

ABaseWindow*& ABaseWindow::currentWindowStorage() {
    thread_local ABaseWindow* threadLocal = nullptr;
    static ABaseWindow* global = nullptr;
    if (threadLocal)
        return threadLocal;
    return global;
}


void ABaseWindow::onFocusLost() {
    AView::onFocusLost();
    closeOverlappingSurfacesOnClick();
}

void ABaseWindow::blockUserInput(bool blockUserInput) {

}

bool ABaseWindow::onDragEnter(const ADragNDrop::EnterEvent& event) {
    return false;
}

void ABaseWindow::onDragLeave() {

}

void ABaseWindow::onDragDrop(const ADragNDrop::DropEvent& event) {

}


void ABaseWindow::requestTouchscreenKeyboard() {
    if (mIgnoreTouchscreenKeyboardRequests) {
        return;
    }
    mIgnoreTouchscreenKeyboardRequests = true;
    requestTouchscreenKeyboardImpl();
}

void ABaseWindow::hideTouchscreenKeyboard() {
    if (mIgnoreTouchscreenKeyboardRequests) {
        return;
    }
    hideTouchscreenKeyboardImpl();
}

bool ABaseWindow::shouldDisplayHoverAnimations() const {
#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS
    return false;
#else
    return isFocused() && !AInput::isKeyDown(AInput::LBUTTON)
           && !AInput::isKeyDown(AInput::CBUTTON)
           && !AInput::isKeyDown(AInput::RBUTTON)
           && !mPreventClickOnPointerRelease;
#endif
}


void ABaseWindow::requestTouchscreenKeyboardImpl() {
    // stub
}

void ABaseWindow::hideTouchscreenKeyboardImpl() {
    // stub
}

void ABaseWindow::preventClickOnPointerRelease() {
    if (mPreventClickOnPointerRelease) {
        return;
    }

    onClickPrevented();

    mPreventClickOnPointerRelease = true;
}
