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

#include "AViewContainer.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AView.h"
#include "AUI/Render/IRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

#include "AUI/Platform/AWindow.h"
#include "AUI/Util/AMetric.h"
#include "AUI/Logging/ALogger.h"
#include "glm/gtc/quaternion.hpp"
#include <AUI/Traits/iterators.h>


static constexpr auto LOG_TAG = "AViewContainer";

namespace aui::view::impl {
    bool isDefinitelyInvisible(AView& view) {
        if (view.getVisibility() == Visibility::INVISIBLE || view.getVisibility() == Visibility::GONE) [[unlikely]] {
            return true;
        }

        // consider anything below this value as effectively "zero" or negligible in terms of opacity.
        if (view.getOpacity() < 0.0001f) [[unlikely]] {
            return true;
        }

        return false;
    }
}

void AViewContainer::drawView(const _<AView>& view, ARenderContext contextOfTheContainer) {
    if (aui::view::impl::isDefinitelyInvisible(*view)) [[unlikely]] {
        return;
    }

    if (view->mSkipUntilLayoutUpdate) [[unlikely]] {
        view->mSkipUntilLayoutUpdate = false;
        return;
    }

    auto contextOfTheView = contextOfTheContainer.withShiftedPosition(-view->getPosition());
    ARect<int> rectOfTheView{ .p1 = {0, 0}, .p2 = view->getSize() };
    if (!ranges::any_of(contextOfTheView.clippingRects, [&](const auto& r) {
        return rectOfTheView.isIntersects(r);
    })) {
        return;
    }

    const auto prevStencilLevel = contextOfTheView.render.getStencilDepth();

    const bool showRedraw = [&] {
        if (view->mRedrawRequested) [[unlikely]] {
            if (auto w = AWindow::current()) [[unlikely]] {
                if (w->profiling().highlightRedrawRequests) {
                    return true;
                }
            }
        }
        return false;
    }();
    AUI_DEFER {
        if (showRedraw) [[unlikely]] {
            auto c = contextOfTheView.render.getColor();
            AUI_DEFER { contextOfTheView.render.setColorForced(c); };
            contextOfTheView.render.rectangle(ASolidBrush{0x40ff00ff_argb}, view->getPosition(), view->getSize());
        }
    };

    RenderHints::PushState s(contextOfTheView.render);
    glm::mat4 t(1.f);
    view->getTransform(t);
    contextOfTheView.render.setTransform(t);
    contextOfTheView.render.setColor(AColor(1, 1, 1, view->getOpacity()));
    if (view->mRenderToTexture) [[unlikely]] { // view was prerendered to texture; see AView::markPixelDataInvalid
        view->mRenderToTexture->skipRedrawUntilTextureIsPresented = false;
        // Check invalidArea is not dirty; otherwise we would have to draw the views without render-to-texture
        // optimizations.
        // Unfortunately, we can't quickly refresh the texture here because aui's main render buffer is already in use
        // and contains uncommited data.
        if (view->mRenderToTexture->drawFromTexture) {
            view->mRenderToTexture->rendererInterface->draw(contextOfTheContainer.render);
            return;
        }
    }
    try {
        view->render(contextOfTheView);
        view->postRender(contextOfTheView);
    }
    catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Unable to render view: " << e;
        contextOfTheView.render.setStencilDepth(prevStencilLevel);
        return;
    }

    {
        auto currentStencilLevel = contextOfTheView.render.getStencilDepth();
        AUI_ASSERT(currentStencilLevel == prevStencilLevel);
    }
}


AViewContainer::AViewContainer() {

}

AViewContainer::~AViewContainer() {
    mLayout = nullptr;
    for (auto& view: mViews) {
        view->mParent = nullptr;
    }
    //Stylesheet::inst().invalidateCache();
}

void AViewContainer::addViews(AVector<_<AView>> views) {
    for (const auto& view: views) {
        view->mParent = this;
        view->mSkipUntilLayoutUpdate = true;
        AUI_NULLSAFE(mLayout)->addView(view);
        view->onViewGraphSubtreeChanged();
    }

    if (mViews.empty()) {
        mViews = std::move(views);
    } else {
        mViews.insertAll(std::move(views));
    }
    invalidateCaches();
    emit childrenChanged;
}

void AViewContainer::addView(const _<AView>& view) {
    AUI_NULLSAFE(view->mParent)->removeView(view);
    view->mSkipUntilLayoutUpdate = true;
    mViews << view;
    view->mParent = this;
    AUI_NULLSAFE(mLayout)->addView(view);
    view->onViewGraphSubtreeChanged();
    invalidateCaches();
    emit childrenChanged;
}

void AViewContainer::addViewCustomLayout(const _<AView>& view) {
    view->mSkipUntilLayoutUpdate = true;
    mViews << view;
    view->mParent = this;
    view->setSize(view->getMinimumSize());
    AUI_NULLSAFE(mLayout)->addView(view);
    view->onViewGraphSubtreeChanged();
    invalidateCaches();
    emit childrenChanged;
}

void AViewContainer::addView(size_t index, const _<AView>& view) {
    view->mSkipUntilLayoutUpdate = true;
    mViews.insert(mViews.begin() + index, view);
    view->mParent = this;
    AUI_NULLSAFE(mLayout)->addView(view, index);
    view->onViewGraphSubtreeChanged();
    invalidateCaches();
    emit childrenChanged;
}

void AViewContainer::setLayout(_<ALayout> layout) {
    for (const auto& v : mViews) {
        v->mParent = nullptr;
    }
    mViews.clear();
    mLayout = std::move(layout);
    if (mLayout) {
        mViews = mLayout->getAllViews();
        for (const auto& v : mViews) {
            v->mParent = this;
            v->onViewGraphSubtreeChanged();
        }
    }
    invalidateCaches();
    emit childrenChanged;
}

void AViewContainer::removeView(const _<AView>& view) {
    auto index = mViews.removeFirst(view);
    if (!index) return;
    view->mParent = nullptr;
    AUI_NULLSAFE(mLayout)->removeView(view, *index);
    invalidateCaches();
    emit childrenChanged;
}

void AViewContainer::removeView(AView* view) {
    auto it = std::find_if(mViews.begin(), mViews.end(), [&](const _<AView>& item) { return item.get() == view; });
    if (it != mViews.end()) {
        if (mLayout) {
            auto sharedPtr = *it;
            auto index = std::distance(mViews.begin(), it);
            mViews.erase(it);
            mLayout->removeView(sharedPtr, index);
        } else {
            mViews.erase(it);
        }
        view->mParent = nullptr;
    }
    invalidateCaches();
    emit childrenChanged;
}

void AViewContainer::removeView(size_t index) {
    auto view = std::move(mViews[index]);
    mViews.removeAt(index);
    if (mLayout)
        mLayout->removeView(view, index);
    view->mParent = nullptr;
    invalidateCaches();
    emit childrenChanged;
}

void AViewContainer::render(ARenderContext context) {
    AView::render(context);
    renderChildren(context);
}

void AViewContainer::onMouseEnter() {
    AView::onMouseEnter();
}

void AViewContainer::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AView::onPointerMove(pos, event);

    auto viewUnderPointer = getViewAt(pos);
    auto targetView = isPressed() ? pointerEventsMapping(event.pointerIndex) : viewUnderPointer;

    if (viewUnderPointer && !viewUnderPointer->isMouseEntered()) {
        viewUnderPointer->onMouseEnter();
    }

    if (targetView) {
        auto mousePos = pos - glm::vec2(targetView->getPosition());
        targetView->onPointerMove(mousePos, event);
    }

    for (auto& v: mViews) {
        if (v->isMouseEntered() && v != viewUnderPointer) {
            v->onMouseLeave();
        }
    }
}

void AViewContainer::onMouseLeave() {
    AView::onMouseLeave();
    for (auto& view: mViews) {
        if (view->isMouseEntered()) {
            view->onMouseLeave();
        }
    }
}

int AViewContainer::getContentMinimumWidth(ALayoutDirection layout) {
    if (mLayout) {
        return (glm::max)(mLayout->getMinimumWidth(), AView::getContentMinimumWidth(ALayoutDirection::NONE));
    }
    return AView::getContentMinimumWidth(ALayoutDirection::NONE);
}


int AViewContainer::getContentMinimumHeight(ALayoutDirection layout) {
    if (mLayout) {
        return (glm::max)(mLayout->getMinimumHeight(), AView::getContentMinimumHeight(ALayoutDirection::NONE));
    }
    return AView::getContentMinimumHeight(ALayoutDirection::NONE);
}

void AViewContainer::onPointerPressed(const APointerPressedEvent& event) {
    AView::onPointerPressed(event);

    //discard focus chain target for proper updating of focus chain targets when moving from child to parent
    mFocusChainTarget.reset();

    auto p = getViewAt(event.position);
    if (p && p->isEnabled()) {
        mPointerEventsMapping.push_back({event.pointerIndex, p, isBlockClicksWhenPressed() && p->isBlockClicksWhenPressed()});
        auto& pointerEvent = mPointerEventsMapping.back();
        pointerEvent.isBlockClicksWhenPressed &= isBlockClicksWhenPressed();
        if (p->capturesFocus()) {
            p->focus(false);

            //updating focus chain targets for views between p and first (maybe indirect) parent view of p that captures focus
            auto childView = p;
            for (auto view = p->getParent(); view; view = view->getParent()) {
                if (view->focusChainTarget() == childView) {
                    break;
                }
                view->setFocusChainTarget(childView);
                childView = view->sharedPtr();
            }
        }
        auto copy = event;
        copy.position -= p->getPosition();
        p->onPointerPressed(copy);
        pointerEvent.isBlockClicksWhenPressed &= p->isBlockClicksWhenPressed();
        if (auto parent = getParent(); parent && !pointerEvent.isBlockClicksWhenPressed) {
            auto it = std::find_if(parent->mPointerEventsMapping.begin(), parent->mPointerEventsMapping.end(),
                                   [&](const auto &parentPointerEvent) {
                                       return parentPointerEvent.pointerIndex == pointerEvent.pointerIndex;
                                   });
            if (it != parent->mPointerEventsMapping.end()) {
                it->isBlockClicksWhenPressed = false;
            }
        }
    }
}

void AViewContainer::onPointerReleased(const APointerReleasedEvent& event) {
    AView::onPointerReleased(event);

    auto viewUnderPointer = getViewAt(event.position);
    auto targetView = pointerEventsMapping(event.pointerIndex);
    if (!targetView) {
        targetView = viewUnderPointer;
    }

    if (targetView && targetView->isEnabled() && targetView->isPressed()) {
        auto copy = event;
        copy.position -= targetView->getPosition();
        copy.triggerClick &= viewUnderPointer == targetView;
        targetView->onPointerReleased(copy);
    }

    mPointerEventsMapping.erase(std::remove_if(mPointerEventsMapping.begin(), mPointerEventsMapping.end(), [&](const PointerEventsMapping& v) {
        return v.pointerIndex == event.pointerIndex;
    }), mPointerEventsMapping.end());
}

void AViewContainer::onPointerDoubleClicked(const APointerPressedEvent& event) {
    AView::onPointerDoubleClicked(event);

    auto p = getViewAt(event.position);
    if (p && p->isEnabled()) {
        auto copy = event;
        copy.position -= p->getPosition();
        p->onPointerDoubleClicked(copy);
    }
}

void AViewContainer::onScroll(const AScrollEvent& event) {
    AView::onScroll(event);
    auto p = getViewAt(event.origin);
    if (p && p->isEnabled()) {
        auto eventCopy = event;
        eventCopy.origin -= p->getPosition();
        p->onScroll(eventCopy);
    }
}

bool AViewContainer::consumesClick(const glm::ivec2& pos) {
    if (mConsumesClickCache) {
        if (mConsumesClickCache->position == pos) {
            return mConsumesClickCache->value;
        }
    }

    AView::consumesClick(pos);

    bool result = false;
    ARaiiHelper onExit = [&] {
        mConsumesClickCache = ConsumesClickCache{
            .position = pos,
            .value = result,
        };
    };

    // has layout check
    if (mAss[int(ass::prop::PropertySlot::BACKGROUND_SOLID)] ||
        mAss[int(ass::prop::PropertySlot::BACKGROUND_IMAGE)]) {
        return result = true;
    }
    if (auto p = getViewAt(pos, AViewLookupFlags::ONLY_THAT_CONSUMES_CLICK)) {
        return result = true;
    }
    return false;
}

_<ALayout> AViewContainer::getLayout() const {
    return mLayout;
}


_<AView> AViewContainer::getViewAt(glm::ivec2 pos, ABitField<AViewLookupFlags> flags) const noexcept {
    _<AView> possibleOutput = nullptr;

    for (auto view: aui::reverse_iterator_wrap(mViews)) {
        auto targetPos = pos - view->getPosition();

        bool hitTest;
        switch (view->getMouseCollisionPolicy()) {
            case MouseCollisionPolicy::DEFAULT:
                hitTest = targetPos.x >= 0 &&
                          targetPos.y >= 0 &&
                          targetPos.x < view->getSize().x &&
                          targetPos.y < view->getSize().y;
                break;

            case MouseCollisionPolicy::MARGIN:
                hitTest = targetPos.x >= -view->getMargin().left &&
                          targetPos.y >= -view->getMargin().top &&
                          targetPos.x < (view->getSize().x + view->getMargin().horizontal()) &&
                          targetPos.y < (view->getSize().y + view->getMargin().vertical());
                break;
        }

        if (hitTest) {
            if (flags.test(AViewLookupFlags::IGNORE_VISIBILITY) || (view->getVisibility() != Visibility::GONE && view->getVisibility() != Visibility::UNREACHABLE)) {
                if (!possibleOutput && !flags.test(AViewLookupFlags::ONLY_THAT_CONSUMES_CLICK)) {
                    possibleOutput = view;
                }
                if (view->consumesClick(targetPos)) {
                    return view;
                }
            }
        }
    }
    return possibleOutput;
}

_<AView> AViewContainer::getViewAtRecursive(glm::ivec2 pos, ABitField<AViewLookupFlags> flags) const noexcept {
    _<AView> target = getViewAt(pos, flags);
    if (!target)
        return nullptr;
    int depth = 0;
    while (auto asContainer = _cast<AViewContainer>(target)) {
        pos -= asContainer->getPosition();
        target = asContainer->getViewAt(pos, flags);
        if (!target)
            return asContainer;
    }
    return target;
}

void AViewContainer::setSize(glm::ivec2 size) {
    mSizeSet = true;
    AView::setSize(size);
    adjustContentSize();
    updateLayout();
}

void AViewContainer::invalidateAllStyles() {
    AView::invalidateAllStyles();
    if (mSizeSet)
        updateLayout();
}

void AViewContainer::updateLayout() {
    if (!mLayout) {
        // no layout = no update.
        return;
    }
    if (!mWantsLayoutUpdate) { // check if this container is part of invalidated min content size chain
        if (mLastLayoutUpdateSize == getSize()) {
            // no need to go deeper.
            return;
        }
    }
    mWantsLayoutUpdate = false;
    mLastLayoutUpdateSize = getSize();
    mLayout->onResize(mPadding.left, mPadding.top,
                      getSize().x - mPadding.horizontal(), getSize().y - mPadding.vertical());
    mConsumesClickCache.reset();
    redraw();
}

void AViewContainer::removeAllViews() {
    if (mLayout) {
        // using reverse iterator wrap here as vector is not efficient in removing first elements
        std::size_t i = getViews().size();
        for (auto& x: aui::reverse_iterator_wrap(getViews())) {
            mLayout->removeView(x, --i);
        }
    }
    mViews.clear();
    invalidateCaches();
}

void AViewContainer::onDpiChanged() {
    AView::onDpiChanged();
    for (auto& v: mViews) {
        v->onDpiChanged();
    }
}

void AViewContainer::setContents(const _<AViewContainer>& container) {
    AUI_ASSERT(("Container passed to setContents should be exact AViewContainer (not derived from). See docs of AViewContainer::setContents" &&
            typeid(*container.get()) == typeid(AViewContainer)));
    setLayout(std::move(container->mLayout));
    mViews = std::move(container->mViews);
    for (auto& v: mViews) {
        v->mParent = this;
    }
    mCustomStyleRule = std::move(container->mCustomStyleRule);
    mAssNames.insertAll(container->mAssNames);
    mAssHelper = nullptr;
    invalidateCaches();
}

void AViewContainer::setEnabled(bool enabled) {
    if (isEnabled() == enabled) [[unlikely]] {
        return;
    }
    AView::setEnabled(enabled);
    notifyParentEnabledStateChanged(enabled);
}

void AViewContainer::notifyParentEnabledStateChanged(bool enabled) {
    enabled &= mDirectlyEnabled;
    AView::notifyParentEnabledStateChanged(enabled);
    for (auto& v: mViews) {
        v->notifyParentEnabledStateChanged(enabled);
    }
}

bool AViewContainer::capturesFocus() {
    return false; // we don't want every single container to catch focus.
}

bool AViewContainer::onGesture(const glm::ivec2& origin, const AGestureEvent& event) {
    auto p = getViewAt(origin);
    if (p && p->isEnabled())
        return p->onGesture(origin - p->getPosition(), event);
    return false;
}

void AViewContainer::invalidateAssHelper() {
    AView::invalidateAssHelper();
    for (const auto& v: mViews) {
        v->invalidateAssHelper();
    }
}

void AViewContainer::onKeyDown(AInput::Key key) {
    AView::onKeyDown(key);
    AUI_NULLSAFE(focusChainTarget())->onKeyDown(key);
}

void AViewContainer::onKeyRepeat(AInput::Key key) {
    AView::onKeyRepeat(key);
    AUI_NULLSAFE(focusChainTarget())->onKeyRepeat(key);
}

void AViewContainer::onKeyUp(AInput::Key key) {
    AView::onKeyUp(key);
    AUI_NULLSAFE(focusChainTarget())->onKeyUp(key);
}

void AViewContainer::onCharEntered(char16_t c) {
    AView::onCharEntered(c);
    AUI_NULLSAFE(focusChainTarget())->onCharEntered(c);
}

void AViewContainer::adjustContentSize() {
    if (mScrollbarAppearance.getVertical() == ScrollbarAppearance::NO_SCROLL_SHOW_CONTENT)
        adjustVerticalSizeToContent();

    if (mScrollbarAppearance.getHorizontal() == ScrollbarAppearance::NO_SCROLL_SHOW_CONTENT)
        adjustHorizontalSizeToContent();
}

void AViewContainer::adjustHorizontalSizeToContent() {
    setFixedSize(glm::ivec2(0, getFixedSize().y));
}

void AViewContainer::adjustVerticalSizeToContent() {
    setFixedSize(glm::ivec2(getFixedSize().x, 0));
}

void AViewContainer::onClickPrevented() {
    AView::onClickPrevented();
    auto pointerEvents = std::move(mPointerEventsMapping);
    for (const auto& e : pointerEvents) {
        if (auto v = e.targetView.lock(); v && v->isEnabled() && v->isPressed(e.pointerIndex)) {
            v->onClickPrevented();
        }
    }
}

void AViewContainer::invalidateCaches() {
    mConsumesClickCache.reset();
    markMinContentSizeInvalid();
    redraw();
}

void AViewContainer::onViewGraphSubtreeChanged() {
    AView::onViewGraphSubtreeChanged();
    for (const auto& v : mViews) {
        v->onViewGraphSubtreeChanged();
    }
}

_<AView> AViewContainer::pointerEventsMapping(APointerIndex index) {
    auto it = std::find_if(mPointerEventsMapping.begin(), mPointerEventsMapping.end(), [&](const PointerEventsMapping& v) {
        return v.pointerIndex == index;
    });
    if (it == mPointerEventsMapping.end()) {
        return nullptr;
    }
    return it->targetView.lock();
}

void AViewContainer::setViews(AVector<_<AView>> views) {
    views.removeIf([](const _<AView>& v) { return v == nullptr; });
    mViews = std::move(views);

    for (const auto& view : mViews) {
        view->mParent = this;
        view->mSkipUntilLayoutUpdate = true;
        if (mLayout)
            mLayout->addView(view);
    }
}

void AViewContainer::markMinContentSizeInvalid() {
    AView::markMinContentSizeInvalid();
    mWantsLayoutUpdate = true;
}
