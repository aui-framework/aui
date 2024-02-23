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
#include "AUI/Event/APointerIndex.h"
#include "AUI/Performance/APerformanceSection.h"
#include "AUI/Render/ABrush.h"
#include "AUI/Util/ARandom.h"
#include "AUI/Platform/AWindow.h"
#include "ABaseWindow.h"
#include <AUI/Action/AMenu.h>
#include <AUI/Traits/memory.h>
#include <AUI/Util/kAUI.h>
#include <chrono>
#include "APlatform.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/View/AViewContainer.h"
#include <AUI/Devtools/DevtoolsPanel.h>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Util/ALayoutInflater.h>
#include <AUI/Util/AViewProfiler.h>
#include <AUI/UITestState.h>

static constexpr auto LOG_TAG = "ABaseWindow";

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

void ABaseWindow::updateFocusChain() {
    if (auto focusedView = mFocusedView.lock()) {
        _weak<AView> focusChainTarget = mFocusedView;
        if (auto container = _cast<AViewContainer>(focusedView)) {
            container->setFocusChainTarget({});
        }

        for (auto target = focusedView->getParent(); target != nullptr; target = target->getParent()) {
            target->setFocusChainTarget(std::move(focusChainTarget));
            focusChainTarget = target->weakPtr();
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
        setFocusedView(target->sharedPtr());
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
#if AUI_PLATFORM_IOS || AUI_PLATFORM_ANDROID
    AWindow::getWindowManager().watchdog().runOperation([&] {
#endif
#if AUI_SHOW_TOUCHES
    if (!isPressed()) {
        // first touch of gesture
        mShowTouches.clear();
    }
    ALogger::info("AUI_SHOW_TOUCHES") << "onPointerPressed(pointerIndex = " << event.pointerIndex << ", pos = " << event.position << ")";
    mShowTouches[event.pointerIndex] = {};
#endif
    mMousePos = event.position;
    closeOverlappingSurfacesOnClick();
    mPreventClickOnPointerRelease = false;
    mPerformDoubleClickOnPointerRelease = false;
    auto focusCopy = mFocusedView.lock();
    mIgnoreTouchscreenKeyboardRequests = false;

    // handle touchscreen scroll
    if (event.pointerIndex.isFinger()) {
        if (auto it = std::find_if(mScrolls.begin(), mScrolls.end(), [&](const Scroll& scroll) {
                return event.pointerIndex == scroll.pointer;
            }); it != mScrolls.end()) {
            mScrolls.erase(it);
        }
        ATouchScroller scroller;
        scroller.handlePointerPressed(event);
        mScrolls.push_back({
            .pointer = event.pointerIndex,
            .scroller = std::move(scroller),
        });
    }

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
    static constexpr auto DOUBLECLICK_RANGE2 = 10_dp;
    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    auto delta = now - mLastButtonPressedTime;
    if (delta < DOUBLECLICK_MAX_DURATION && glm::distance2(mLastPosition, event.position) <= DOUBLECLICK_RANGE2.getValuePx()) {
        if (mLastButtonPressed == event.pointerIndex) {
            mPerformDoubleClickOnPointerRelease = true;
            mLastButtonPressedTime = 0ms;
        }
    } else {
        mLastButtonPressedTime = now;
        mLastButtonPressed = event.pointerIndex;
        mLastPosition = event.position;
    }
    AMenu::close();
#if AUI_PLATFORM_IOS || AUI_PLATFORM_ANDROID
    });
#endif
}

void ABaseWindow::onPointerReleased(const APointerReleasedEvent& event) {
#if AUI_PLATFORM_IOS || AUI_PLATFORM_ANDROID
    AWindow::getWindowManager().watchdog().runOperation([&] {
#endif
#if AUI_SHOW_TOUCHES
    ALogger::info("AUI_SHOW_TOUCHES") << "onPointerReleased(pI=" << event.pointerIndex << ",pos=" << event.position << ",tC=" << event.triggerClick << ")";
#endif
    APointerReleasedEvent copy = event;
    // in case of multitouch, we should not treat pointer release event as a click.
    copy.triggerClick = pointerEventsMapping().size() < 2 && !mPreventClickOnPointerRelease.valueOr(true);
    mPreventClickOnPointerRelease.reset();

    // handle touchscreen scroll
    if (event.pointerIndex.isFinger()) {
        if (auto it = std::find_if(mScrolls.begin(), mScrolls.end(), [&](const Scroll& scroller) {
                return event.pointerIndex == scroller.pointer;
            }); it != mScrolls.end()) {
            it->scroller.handlePointerReleased(event);
        } else {
            ALogger::warn(LOG_TAG) << "ABaseWindow::onPointerReleased is unable to find finger " << event.pointerIndex;
        }
    }

    AViewContainer::onPointerReleased(copy);
    if (mPerformDoubleClickOnPointerRelease) {
        onPointerDoubleClicked({
            .position = event.position,
            .pointerIndex = event.pointerIndex,
            .asButton = event.asButton
        });
    }
    mPerformDoubleClickOnPointerRelease = false;

#if AUI_PLATFORM_IOS || AUI_PLATFORM_ANDROID
    });
#endif
}

void ABaseWindow::forceUpdateCursor() {
    AViewContainer::onPointerMove(mMousePos, {});
}

void ABaseWindow::onScroll(const AScrollEvent& event) {
    AViewContainer::onScroll(event);
    AViewContainer::onPointerMove(mMousePos, {event.pointerIndex}); // update hovers inside scrollarea
}

void ABaseWindow::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
#if AUI_PLATFORM_IOS || AUI_PLATFORM_ANDROID
    AWindow::getWindowManager().watchdog().runOperation([&] {
#endif
#if AUI_SHOW_TOUCHES
    ALogger::info("AUI_SHOW_TOUCHES") << "onPointerMove(pI=" << event.pointerIndex << ",pos=" << pos << ")";
    if (auto c = mShowTouches.contains(event.pointerIndex)) {
        c->second.positions << pos;
    }
#endif
    mMousePos = pos;
    mCursor = ACursor::DEFAULT;

    // handle touchscreen scroll
    if (event.pointerIndex.isFinger()) {
        if (auto it = std::find_if(mScrolls.begin(), mScrolls.end(), [&](const Scroll& scroller) {
                return event.pointerIndex == scroller.pointer;
            }); it != mScrolls.end()) {
            auto d = it->scroller.handlePointerMove(pos);
            if (d != glm::ivec2(0, 0)) {
                onScroll(AScrollEvent {
                    .origin       = it->scroller.origin(),
                    .delta        = d,
                    .kinetic      = false,
                    .pointerIndex = event.pointerIndex,
                });
            }
        } else {
            ALogger::warn(LOG_TAG) << "ABaseWindow::onPointerMove is unable to find finger " << event.pointerIndex;
        }
    }

    AViewContainer::onPointerMove(pos, event);

    emit mouseMove(pos);
#if AUI_PLATFORM_IOS || AUI_PLATFORM_ANDROID
    });
#endif
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

void ABaseWindow::updateLayout() {
    APerformanceSection updateLayout("layout update");
    AViewContainer::updateLayout();
}

void ABaseWindow::render(ClipOptimizationContext context) {
    APerformanceSection root("render");
#if AUI_PLATFORM_IOS || AUI_PLATFORM_ANDROID
    AWindow::getWindowManager().watchdog().runOperation([&] {
#endif
    mScrolls.erase(std::remove_if(mScrolls.begin(), mScrolls.end(), [&](Scroll& scroll) {
        auto delta = scroll.scroller.gatherKineticScrollValue();
        if (!delta) {
            return false;
        }
        if (*delta == glm::ivec2(0, 0)) {
            return true;
        }
        onScroll(AScrollEvent {
                .origin       = scroll.scroller.origin(),
                .delta        = *delta,
                .kinetic      = true,
                .pointerIndex = scroll.pointer,
        });
        flagRedraw();
        return false;
    }), mScrolls.end());

    AViewContainer::render(context);
    mIgnoreTouchscreenKeyboardRequests = false;

    if (auto v = mProfiledView.lock()) {
        AViewProfiler::displayBoundsOn(*v);
    }

#if AUI_SHOW_TOUCHES
    for (const auto&[pointerIndex, data] : mShowTouches) {
        ARender::lines(ASolidBrush{AColor::BLUE}, data.positions);
        ARender::points(ASolidBrush{AColor::RED}, data.positions, 6_dp);

        if (!isPressed(pointerIndex) && data.positions.size() >= 2) {
            // show tangent trail
            const auto last = data.positions.last();
            const auto preLast = *(data.positions.end() - 2);
            const auto vector = last - preLast;

            const glm::vec2 trail[] = {
                last,
                last + vector * 10.f / float(frameMillis()),
            };
            ARender::lines(ASolidBrush{AColor::GREEN}, trail);
        }
    }
#endif

    using namespace std::chrono;
    using namespace std::chrono_literals;
    auto now = high_resolution_clock::now();
    ++mFpsCounter;
    if (auto delta = duration_cast<microseconds>(now - mLastTimeFpsCaptured).count();
        delta >= duration_cast<microseconds>(1s).count()) {
        mLastCapturedFps = duration_cast<microseconds>(1s).count() * (mFpsCounter) / delta;
        mFpsCounter = 0;
        mLastTimeFpsCaptured = now;
    }
#if AUI_PLATFORM_IOS || AUI_PLATFORM_ANDROID
    });
#endif
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
           && !isPreventingClickOnPointerRelease();
#endif
}


void ABaseWindow::requestTouchscreenKeyboardImpl() {
    // stub
}

void ABaseWindow::hideTouchscreenKeyboardImpl() {
    // stub
}

void ABaseWindow::preventClickOnPointerRelease() {
    if (!mPreventClickOnPointerRelease) {
        return;
    }
    if (mPreventClickOnPointerRelease.value()) {
        return;
    }
    AViewContainer::onClickPrevented();
    mPreventClickOnPointerRelease = true;
}

bool ABaseWindow::onGesture(const glm::ivec2& origin, const AGestureEvent& event) {
    bool v = AViewContainer::onGesture(origin, event);
    if (v) {
        preventClickOnPointerRelease();
    }
    return v;
}
