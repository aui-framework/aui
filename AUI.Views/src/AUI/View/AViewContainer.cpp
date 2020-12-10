#include "AViewContainer.h"
#include "AView.h"
#include "AUI/Render/Render.h"
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

#include "AUI/Platform/AWindow.h"
#include "AUI/Util/AMetric.h"


unsigned char stencilDepth = 0;

void AViewContainer::drawView(const _<AView>& view)
{
	if (view->getVisibility() == V_VISIBLE) {
		RenderHints::PushState s;
		glm::mat4 t(1.f);
		view->getTransform(t);
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


void AViewContainer::userProcessStyleSheet(
	const std::function<void(css, const std::function<void(property)>&)>& processor)
{
	processor(css::T_AUI_SPACING, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			mLayout->setSpacing(AMetric(p->getArgs()[0]).getValuePx());
		}
	});

	mHasBackground = false;
	auto bg = [&](property p)
	{
		if (p->getArgs().size() > 0 && p->getArgs()[0] != "none") {
			mHasBackground = true;
		}
	};
	processor(css::T_BACKGROUND, bg);
	processor(css::T_BACKGROUND_COLOR, bg);
}

AViewContainer::AViewContainer()
{
	AVIEW_CSS;
}

AViewContainer::~AViewContainer()
{
	//Stylesheet::inst().invalidateCache();
}

void AViewContainer::addView(_<AView> view)
{
	mViews << view;
	view->mParent = this;
	if (mLayout)
		mLayout->addView(view);
}

void AViewContainer::removeView(_<AView> view)
{
	mViews.remove(view);
	if (mLayout)
		mLayout->removeView(view);
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

	for (auto& view : mViews)
	{
		if (!view->isEnabled())
			continue;
		
		auto mousePos = pos - view->getPosition();
		bool hovered = mousePos.x >= 0 && mousePos.y >= 0 && mousePos.x < view->getSize().x && mousePos.y < view->getSize().y;
        if (view->isMouseHover() != hovered) {
            if (hovered) {
                view->onMouseEnter();
            } else {

                view->onMouseLeave();
            }
        }
		if (hovered)
		{
			view->onMouseMove(mousePos);
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
		return mLayout->getMinimumWidth();
	}
	return AView::getContentMinimumWidth();
}

int AViewContainer::getContentMinimumHeight()
{
	if (mLayout)
	{
		return mLayout->getMinimumHeight();
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
	if (mHasBackground)
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
	for (auto it = mViews.rbegin(); it != mViews.rend(); ++it)
	{
		auto view = *it;
		auto targetPos = pos - view->getPosition();

		if (targetPos.x >= 0 && targetPos.y >= 0 && targetPos.x < view->getSize().x && targetPos.y < view->getSize().y)
		{
			if (!ignoreGone || view->getVisibility() != V_GONE) {
			    if (!possibleOutput)
			        possibleOutput = view;
                if (view->consumesClick(targetPos))
			        return view;
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
            mLayout->removeView(x);
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
