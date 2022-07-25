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
#include "Platform.h"
#include <AUI/Devtools/DevtoolsPanel.h>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Util/ALayoutInflater.h>
#include <AUI/Util/AViewProfiler.h>
#include <AUI/UITestState.h>

ABaseWindow::ABaseWindow() {
    mDpiRatio = Platform::getDpiRatio();
}

ABaseWindow::~ABaseWindow() {
    if (currentWindowStorage() == this) {
        currentWindowStorage() = nullptr;
    }
}

float ABaseWindow::fetchDpiFromSystem() const {
    return Platform::getDpiRatio();
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

void ABaseWindow::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    closeOverlappingSurfacesOnClick();
    auto focusCopy = mFocusedView.lock();
    AViewContainer::onMousePressed(pos, button);
    if (mFocusedView.lock() != focusCopy && focusCopy != nullptr) {
        if (focusCopy->hasFocus()) {
            focusCopy->onFocusLost();
        }
    }

    // check for double clicks
    using namespace std::chrono;
    using namespace std::chrono_literals;
    static milliseconds lastButtonPressedTime = 0ms;
    static AInput::Key lastButtonPressed = AInput::UNKNOWN;
    static glm::ivec2 lastPosition = {0, 0};

    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    auto delta = now - lastButtonPressedTime;
    if (delta < 500ms && lastPosition == pos) {
        if (lastButtonPressed == button) {
            onMouseDoubleClicked(pos, button);

            lastButtonPressedTime = 0ms;
        }
    } else {
        lastButtonPressedTime = now;
        lastButtonPressed = button;
        lastPosition = pos;
    }
    AMenu::close();
}

void ABaseWindow::onMouseReleased(glm::ivec2 pos, AInput::Key button) {
    AViewContainer::onMouseReleased(pos, button);
}

void ABaseWindow::onMouseMove(glm::ivec2 pos) {
    mMousePos = pos;
    AViewContainer::onMouseMove(pos);
    auto v = getViewAtRecursive(pos);
    if (v) {
        mCursor = v->getCursor();
    }
    if (!AWindow::shouldDisplayHoverAnimations()) {
        if (auto focused = mFocusedView.lock()) {
            if (focused != v) {
                focused->onMouseMove(pos - focused->getPositionInWindow());
            }
        }
    }
    emit mouseMove(pos);
}

void ABaseWindow::onKeyDown(AInput::Key key) {
    emit keyDown(key);
    if (auto v = getFocusedView())
        v->onKeyDown(key);

#ifdef AUI_DEBUG
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

void ABaseWindow::onKeyUp(AInput::Key key) {
    if (auto v = getFocusedView())
        v->onKeyUp(key);
}

void ABaseWindow::onCharEntered(wchar_t c) {
    if (auto v = getFocusedView()) {
        v->onCharEntered(c);
    }
}

void ABaseWindow::onFocusLost() {
    AView::onFocusLost();
    closeOverlappingSurfacesOnClick();
}

void ABaseWindow::blockUserInput(bool blockUserInput) {

}
