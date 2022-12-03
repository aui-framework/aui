// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
#include <AUI/Traits/iterators.h>


void AViewContainer::drawView(const _<AView>& view)
{
	if (view->getVisibility() == Visibility::VISIBLE) {
		RenderHints::PushState s;
		glm::mat4 t(1.f);
		view->getTransform(t);
		Render::setColor(AColor(1, 1, 1, view->getOpacity()));
        Render::setTransform(t);

		try {
			view->render();
		}
		catch (...) {}
		try {
			view->postRender();
		}
		catch (...) {}
	}
}



AViewContainer::AViewContainer()
{

}

AViewContainer::~AViewContainer()
{
	for (auto& view : mViews) {
		view->mParent = nullptr;
	}
	//Stylesheet::inst().invalidateCache();
}

void AViewContainer::addView(const _<AView>& view)
{
	mViews << view;
	view->mParent = this;
	if (mLayout)
        mLayout->addView(-1, view);
}
void AViewContainer::addViewCustomLayout(const _<AView>& view)
{
	mViews << view;
	view->mParent = this;
}
void AViewContainer::addView(size_t index, const _<AView>& view)
{
	mViews.insert(mViews.begin() + index, view);
	view->mParent = this;
	if (mLayout)
        mLayout->addView(index, view);
}

void AViewContainer::removeView(const _<AView>& view)
{
	mViews.removeFirst(view);
	if (mLayout)
        mLayout->removeView(-1, view);
}

void AViewContainer::removeView(AView* view) {
    auto it = std::find_if(mViews.begin(), mViews.end(), [&](const _<AView>& item) { return item.get() == view;});
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

void AViewContainer::removeView(size_t index)
{
	mViews.removeAt(index);
	if (mLayout)
        mLayout->removeView(index, nullptr);
}

void AViewContainer::render()
{
	AView::render();
	drawViews(mViews.begin(), mViews.end());
}

void AViewContainer::onMouseEnter()
{
	AView::onMouseEnter();
}

void AViewContainer::onMouseMove(glm::ivec2 pos)
{
	AView::onMouseMove(pos);

	auto targetView = getViewAt(pos);

	if (targetView) {
        auto mousePos = pos - targetView->getPosition();
	    targetView->onMouseEnter();
        targetView->onMouseMove(mousePos);
	}

	for (auto& v : mViews) {
	    if (v->isMouseHover() && v != targetView) {
	        v->onMouseLeave();
	    }
	}
}

void AViewContainer::onMouseLeave()
{
	AView::onMouseLeave();
	for (auto& view : mViews)
	{
		if (view->isMouseHover() && view->isEnabled())
			view->onMouseLeave();
	}
}

int AViewContainer::getContentMinimumWidth(ALayoutDirection layout)
{
	if (mLayout)
	{
		return (glm::max)(mLayout->getMinimumWidth(), AView::getContentMinimumWidth(ALayoutDirection::NONE));
	}
	return AView::getContentMinimumWidth(ALayoutDirection::NONE);
}


int AViewContainer::getContentMinimumHeight(ALayoutDirection layout)
{
	if (mLayout)
	{
        return (glm::max)(mLayout->getMinimumHeight(), AView::getContentMinimumHeight(ALayoutDirection::NONE));
	}
	return AView::getContentMinimumHeight(ALayoutDirection::NONE);
}

void AViewContainer::onMousePressed(glm::ivec2 pos, AInput::Key button)
{
	AView::onMousePressed(pos, button);

	auto p = getViewAt(pos);
	if (p && p->isEnabled()) {
		p->focus();
		p->onMousePressed(pos - p->getPosition(), button);
	}
}
void AViewContainer::onMouseReleased(glm::ivec2 pos, AInput::Key button)
{
	AView::onMouseReleased(pos, button);
	auto p = getViewAt(pos);
	if (p && p->isEnabled() && p->isMousePressed())
		p->onMouseReleased(pos - p->getPosition(), button);
}

void AViewContainer::onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button)
{
	AView::onMouseDoubleClicked(pos, button);

	auto p = getViewAt(pos);
	if (p && p->isEnabled())
		p->onMouseDoubleClicked(pos - p->getPosition(), button);
}

void AViewContainer::onMouseWheel(glm::ivec2 pos, glm::ivec2 delta) {
    AView::onMouseWheel(pos, delta);
    auto p = getViewAt(pos);
    if (p && p->isEnabled())
        p->onMouseWheel(pos - p->getPosition(), delta);
}

bool AViewContainer::consumesClick(const glm::ivec2& pos) {
    // has layout check
	if (mAss[int(ass::decl::DeclarationSlot::BACKGROUND_SOLID)] || mAss[int(ass::decl::DeclarationSlot::BACKGROUND_IMAGE)])
		return true;
    auto p = getViewAt(pos);
    if (p)
        return p->consumesClick(pos - p->getPosition());
    return false;
}

void AViewContainer::setLayout(_<ALayout> layout)
{
	mViews.clear();
	mLayout = std::move(layout);
}

_<ALayout> AViewContainer::getLayout() const
{
	return mLayout;
}


_<AView> AViewContainer::getViewAt(glm::ivec2 pos, bool ignoreGone)
{
    _<AView> possibleOutput = nullptr;

	for (auto view : aui::reverse_iterator_wrap(mViews))
	{
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

		if (hitTest)
		{
			if (!ignoreGone || view->getVisibility() != Visibility::GONE) {
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

_<AView> AViewContainer::getViewAtRecursive(glm::ivec2 pos)
{
	_<AView> target = getViewAt(pos);
	if (!target)
		return nullptr;
	while (auto parent = _cast<AViewContainer>(target))
	{
		pos -= parent->getPosition();
		target = parent->getViewAt(pos);
		if (!target)
			return parent;
	}
	return target;
}

void AViewContainer::setSize(glm::ivec2 size)
{
    mSizeSet = true;
    AView::setSize(size);
    updateLayout();
}

void AViewContainer::invalidateAllStyles() {
    AView::invalidateAllStyles();
    if (mSizeSet)
        updateLayout();
}

void AViewContainer::updateLayout()
{
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
        for (auto& x : getViews()) {
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
    for (auto& v : mViews) {
        v->onDpiChanged();
    }
}
void AViewContainer::setContents(const _<AViewContainer>& container) {
    assert(("Container passed to setContents should be exact AViewContainer (not derived from). See docs of AViewContainer::setContents" && typeid(*container.get()) == typeid(AViewContainer)));
    setLayout(std::move(container->mLayout));
    mViews = std::move(container->mViews);
    for (auto& v : mViews) {
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
    for (auto& v : mViews) {
        v->notifyParentEnabledStateChanged(enabled);
    }
}

void AViewContainer::focus() {
    // we don't want to focus containers.
}

bool AViewContainer::onGesture(const glm::ivec2& origin, const AGestureEvent& event) {
    auto p = getViewAt(origin);
    if (p && p->isEnabled())
        return p->onGesture(origin - p->getPosition(), event);
    return false;
}

void AViewContainer::invalidateAssHelper() {
    AView::invalidateAssHelper();
    for (const auto& v : mViews) {
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
