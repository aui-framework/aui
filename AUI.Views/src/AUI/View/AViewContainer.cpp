/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#include "AViewContainer.h"
#include "AView.h"
#include "AUI/Render/Render.h"
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

#include "AUI/Platform/AWindow.h"
#include "AUI/Util/AMetric.h"
#include <AUI/Traits/iterators.h>


unsigned char stencilDepth = 0;

void AViewContainer::drawView(const _<AView>& view)
{
	if (view->getVisibility() == Visibility::VISIBLE) {
		RenderHints::PushState s;
		glm::mat4 t(1.f);
		view->getTransform(t);
		Render::inst().setColor(AColor(1, 1, 1, view->getOpacity()));
        Render::inst().setTransform(t);

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
	mViews.remove(view);
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

int AViewContainer::getContentMinimumWidth()
{
	if (mLayout)
	{
		return (glm::max)(mLayout->getMinimumWidth(), AView::getContentMinimumWidth());
	}
	return AView::getContentMinimumWidth();
}


int AViewContainer::getContentMinimumHeight()
{
	if (mLayout)
	{
        return (glm::max)(mLayout->getMinimumHeight(), AView::getContentMinimumHeight());
	}
	return AView::getContentMinimumHeight();
}

void AViewContainer::onMousePressed(glm::ivec2 pos, AInput::Key button)
{
	AView::onMousePressed(pos, button);

	auto p = getViewAt(pos);
	if (p && p->isEnabled()) {
		AWindow::current()->setFocusedView(p);
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

void AViewContainer::onMouseWheel(glm::ivec2 pos, int delta) {
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

		if (targetPos.x >= 0 && targetPos.y >= 0 && targetPos.x < view->getSize().x && targetPos.y < view->getSize().y)
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

void AViewContainer::setSize(int width, int height)
{
    mSizeSet = true;
    AView::setSize(width, height);
    updateLayout();
}

void AViewContainer::recompileCSS() {
    AView::recompileCSS();
    if (mSizeSet)
        updateLayout();
}

void AViewContainer::updateLayout()
{
    if (mLayout)
        mLayout->onResize(mPadding.left, mPadding.top,
                          getSize().x - mPadding.horizontal(), getSize().y - mPadding.vertical());
    updateParentsLayoutIfNecessary();
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
    setLayout(std::move(container->mLayout));
    mViews = std::move(container->mViews);
    for (auto& v : mViews) {
        v->mParent = this;
    }
    mAssNames = std::move(container->mAssNames);
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
