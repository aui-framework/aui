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

#include "AViewContainer.h"
#include "AView.h"
#include "AUI/Render/Render.h"
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

#include "AUI/Platform/AWindow.h"
#include "AUI/Util/AMetric.h"
#include "AUI/Logging/ALogger.h"
#include <AUI/Traits/iterators.h>


static constexpr auto LOG_TAG = "AViewContainer";

void AViewContainer::drawView(const _<AView>& view) {
    if (view->getVisibility() == Visibility::VISIBLE || view->getVisibility() == Visibility::UNREACHABLE) {
        const auto prevStencilLevel = Render::getRenderer()->getStencilDepth();

        RenderHints::PushState s;
        glm::mat4 t(1.f);
        view->getTransform(t);
        Render::setColor(AColor(1, 1, 1, view->getOpacity()));
        Render::setTransform(t);

        try {
            view->render();
            view->postRender();
        }
        catch (const AException& e) {
            ALogger::err(LOG_TAG) << "Unable to render view: " << e;
            Render::getRenderer()->setStencilDepth(prevStencilLevel);
            return;
        }

        auto currentStencilLevel = Render::getRenderer()->getStencilDepth();
        assert(currentStencilLevel == prevStencilLevel);
    }
}


AViewContainer::AViewContainer() {

}

AViewContainer::~AViewContainer() {
    for (auto& view: mViews) {
        view->mParent = nullptr;
    }
    //Stylesheet::inst().invalidateCache();
}

void AViewContainer::addViews(AVector<_<AView>> views) {
    for (const auto& view: views) {
        view->mParent = this;
        AUI_NULLSAFE(mLayout)->addView(-1, view);
        emit view->addedToContainer();
    }

    if (mViews.empty()) {
        mViews = std::move(views);
    } else {
        mViews.insertAll(std::move(views));
    }
}

void AViewContainer::addView(const _<AView>& view) {
    mViews << view;
    view->mParent = this;
    AUI_NULLSAFE(mLayout)->addView(-1, view);
    emit view->addedToContainer();
}

void AViewContainer::addViewCustomLayout(const _<AView>& view) {
    mViews << view;
    view->mParent = this;
    view->setSize(view->getMinimumSize());
    emit view->addedToContainer();
}

void AViewContainer::addView(size_t index, const _<AView>& view) {
    mViews.insert(mViews.begin() + index, view);
    view->mParent = this;
    if (mLayout)
        mLayout->addView(index, view);
    emit view->addedToContainer();
}

void AViewContainer::removeView(const _<AView>& view) {
    mViews.removeFirst(view);
    if (mLayout)
        mLayout->removeView(-1, view);
}

void AViewContainer::removeView(AView* view) {
    auto it = std::find_if(mViews.begin(), mViews.end(), [&](const _<AView>& item) { return item.get() == view; });
    if (it != mViews.end()) {
        if (mLayout) {
            auto sharedPtr = *it;
            mViews.erase(it);
            mLayout->removeView(-1, sharedPtr);
        } else {
            mViews.erase(it);
        }
    }
}

void AViewContainer::removeView(size_t index) {
    mViews.removeAt(index);
    if (mLayout)
        mLayout->removeView(index, nullptr);
}

void AViewContainer::render() {
    AView::render();
    drawViews(mViews.begin(), mViews.end());
}

void AViewContainer::onMouseEnter() {
    AView::onMouseEnter();
}

void AViewContainer::onPointerMove(glm::ivec2 pos) {
    AView::onPointerMove(pos);

    auto viewUnderCursor = getViewAt(pos);
    auto targetView = isMousePressed() ? mFocusChainTarget.lock() : viewUnderCursor;

    if (targetView) {
        auto mousePos = pos - targetView->getPosition();
        if (!targetView->isMouseHover()) targetView->onMouseEnter();
        targetView->onPointerMove(mousePos);
    }

    for (auto& v: mViews) {
        if (v->isMouseHover() && v != viewUnderCursor) {
            v->onMouseLeave();
        }
    }
}

void AViewContainer::onMouseLeave() {
    AView::onMouseLeave();
    for (auto& view: mViews) {
        if (view->isMouseHover())
            view->onMouseLeave();
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

    auto p = getViewAt(event.position);
    if (p && p->isEnabled()) {
        if (p->capturesFocus()) {
            p->focus(false);
        }
        auto copy = event;
        copy.position -= p->getPosition();
        p->onPointerPressed(copy);
        mFocusChainTarget = p;
    }
}

void AViewContainer::onPointerReleased(const APointerReleasedEvent& event) {
    AView::onPointerReleased(event);
    auto chainTarget = mFocusChainTarget.lock();

    if (chainTarget && chainTarget->isEnabled() && chainTarget->isMousePressed()) {
        auto copy = event;
        copy.position -= chainTarget->getPosition();
        copy.triggerClick &= getViewAt(event.position) == chainTarget;
        chainTarget->onPointerReleased(copy);
    }
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
    // has layout check
    if (mAss[int(ass::prop::PropertySlot::BACKGROUND_SOLID)] ||
        mAss[int(ass::prop::PropertySlot::BACKGROUND_IMAGE)])
        return true;
    auto p = getViewAt(pos);
    if (p)
        return p->consumesClick(pos - p->getPosition());
    return false;
}

void AViewContainer::setLayout(_<ALayout> layout) {
    mViews.clear();
    mLayout = std::move(layout);
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
                if (!possibleOutput) {
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
    while (auto parent = _cast<AViewContainer>(target)) {
        pos -= parent->getPosition();
        target = parent->getViewAt(pos, flags);
        if (!target)
            return parent;
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
    /*
    if (getContentMinimumWidth() > getContentWidth() ||
        getContentMinimumHeight() > getContentHeight()) {
        //AWindow::current()->flagUpdateLayout();
    } else {
    }*/
    if (mLayout)
        mLayout->onResize(mPadding.left, mPadding.top,
                          getSize().x - mPadding.horizontal(), getSize().y - mPadding.vertical());
}

void AViewContainer::removeAllViews() {
    if (mLayout) {
        for (auto& x: getViews()) {
            mLayout->removeView(0, x);
        }
    }
    mViews.clear();
}

void AViewContainer::updateParentsLayoutIfNecessary() {
    if (mPreviousSize != mSize) {
        mPreviousSize = mSize;
        if (mParent) {
            mParent->updateLayout();
        }
    }
}

void AViewContainer::onDpiChanged() {
    AView::onDpiChanged();
    for (auto& v: mViews) {
        v->onDpiChanged();
    }
}

void AViewContainer::setContents(const _<AViewContainer>& container) {
    assert(("Container passed to setContents should be exact AViewContainer (not derived from). See docs of AViewContainer::setContents" &&
            typeid(*container.get()) == typeid(AViewContainer)));
    setLayout(std::move(container->mLayout));
    mViews = std::move(container->mViews);
    for (auto& v: mViews) {
        v->mParent = this;
    }
    mCustomStyleRule = std::move(container->mCustomStyleRule);
    mAssNames.insertAll(container->mAssNames);
    mAssHelper = nullptr;
}

void AViewContainer::setEnabled(bool enabled) {
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

void AViewContainer::onCharEntered(wchar_t c) {
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
    AUI_NULLSAFE(mFocusChainTarget.lock())->onClickPrevented();
}
