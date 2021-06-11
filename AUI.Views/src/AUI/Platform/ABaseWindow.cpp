//
// Created by alex2 on 6/9/2021.
//

#include <AUI/Util/AError.h>
#include <AUI/Traits/strings.h>
#include "AUI/Util/ARandom.h"
#include "AUI/Platform/AWindow.h"
#include "ABaseWindow.h"
#include <AUI/Action/AMenu.h>
#include <AUI/Traits/memory.h>
#include <AUI/Util/kAUI.h>
#include <chrono>
#include "Platform.h"
#include <glm/gtc/matrix_transform.hpp>

ABaseWindow::ABaseWindow() {
    mDpiRatio = Platform::getDpiRatio();
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
                    auto index = parentViews.indexOf(target) + 1;
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
        mFocusedView = target->determineSharedPointer();
        if (target) {
            if (!target->hasFocus()) {
                target->onFocusAcquired();
            }
        }
    }
}

void ABaseWindow::onMousePressed(glm::ivec2 pos, AInput::Key button) {
    auto focusCopy = mFocusedView.lock();
    mFocusedView.reset();
    assert(mFocusedView.lock() == nullptr);
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
    static AInput::Key lastButtonPressed = AInput::Unknown;
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

void ABaseWindow::onMouseMove(glm::ivec2 pos) {
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

}

void ABaseWindow::onKeyDown(AInput::Key key) {
    emit keyDown(key);
    if (auto v = getFocusedView())
        v->onKeyDown(key);
}

void ABaseWindow::flagRedraw() {

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