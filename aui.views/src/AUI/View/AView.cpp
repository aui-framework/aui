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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "AView.h"
#include "AUI/Render/Render.h"
#include "AUI/Util/ATokenizer.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Url/AUrl.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/Animator/AAnimator.h"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <AUI/IO/AStringStream.h>
#include <AUI/Util/kAUI.h>
#include <AUI/ASS/AStylesheet.h>
#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Traits/callables.h>

#include "AUI/Platform/ADesktop.h"
#include "AUI/Platform/AFontManager.h"
#include "AUI/Util/AMetric.h"
#include "AUI/Util/Factory.h"
#include "ALabel.h"

// windows.h
#undef max
#undef min


ABaseWindow* AView::getWindow()
{

    AView* parent = nullptr;

    for (AView* target = this; target; target = target->mParent) {
        parent = target;
    }

    return dynamic_cast<ABaseWindow*>(parent);
}

AView::AView()
{

    aui::zero(mAss);
}

void AView::redraw()
{

    nullsafe(getWindow())->flagRedraw(); else nullsafe(AWindow::current())->flagRedraw();

}
void AView::requestLayoutUpdate()
{

    nullsafe(getWindow())->flagUpdateLayout(); else nullsafe(AWindow::current())->flagUpdateLayout();

}

void AView::drawStencilMask()
{
    if (mBorderRadius > 0 && mPadding.horizontal() == 0 && mPadding.vertical() == 0) {
        Render::roundedRect(ASolidBrush{},
                            {mPadding.left, mPadding.top},
                            {getWidth() - mPadding.horizontal(), getHeight() - mPadding.vertical()},
                            mBorderRadius);
    } else {
        Render::rect(ASolidBrush{},
                     {mPadding.left, mPadding.top},
                     {getWidth() - mPadding.horizontal(), getHeight() - mPadding.vertical()});
    }
}

void AView::postRender() {
    if (mAnimator)
        mAnimator->postRender(this);
    popStencilIfNeeded();
}

void AView::popStencilIfNeeded() {
    if (getOverflow() == Overflow::HIDDEN)
    {
        /*
         * If the AView's Overflow set to Overflow::HIDDEN AView pushed it's mask into the stencil buffer but AView
         * cannot return stencil buffer to the previous state by itself because of C++ restrictions. We should also
         * apply mask AFTER transform updated and BEFORE rendering AView content. The only way to return the stencil
         * back is place it here, after rendering AView.
         */
        RenderHints::popMask([&]() {
            drawStencilMask();
        });
    }
}
void AView::render()
{
    if (mAnimator)
        mAnimator->animate(this);

    {
        ensureAssUpdated();

        for (auto& e : mBackgroundEffects)
        {
            e->draw([&]()
                    {
                        Render::rect(ASolidBrush{},
                                     {0, 0},
                                     getSize());
                    });
        }

        // draw list
        for (auto& w : mAss) {
            if (w) {
                w->renderFor(this);
            }
        }
    }

    // stencil
    if (mOverflow == Overflow::HIDDEN)
    {
        RenderHints::pushMask([&]() {
            drawStencilMask();
        });
    }
}

void AView::recompileAss()
{
    mCursor = ACursor::DEFAULT;
    mOverflow = Overflow::VISIBLE;
    mMargin = {};
    mMinSize = {};
    mBorderRadius = 0.f;
    //mForceStencilForBackground = false;
    mMaxSize = glm::ivec2(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    mFontStyle = {};
    mBackgroundEffects.clear();

    for (auto& r : AStylesheet::inst().getRules()) {
        if (r.getSelector().isPossiblyApplicable(this)) {
            mAssHelper->mPossiblyApplicableRules << &r;
            r.getSelector().setupConnections(this, mAssHelper);
        }
    }
    updateAssState();
}

void AView::updateAssState() {
    aui::zero(mAss);
    if (!mAssHelper) return;
    mAssHelper->state.backgroundCropping.size.reset();
    mAssHelper->state.backgroundCropping.offset.reset();
    mAssHelper->state.backgroundImage.reset();
    mAssHelper->state.backgroundUrl.dpiMargin.reset();
    mAssHelper->state.backgroundUrl.overlayColor.reset();
    mAssHelper->state.backgroundUrl.rep.reset();
    mAssHelper->state.backgroundUrl.scale.reset();
    mAssHelper->state.backgroundUrl.sizing.reset();
    mAssHelper->state.backgroundUrl.url.reset();


    for (auto& r : mAssHelper->mPossiblyApplicableRules) {
        if (r->getSelector().isStateApplicable(this)) {
            applyAssRule(* r);
        }
    }
    applyAssRule(mCustomAssRule);

    redraw();
}

/*
void AView::userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor)
{
}
*/

float AView::getTotalFieldHorizontal() const
{
    return mPadding.horizontal() + mMargin.horizontal();
}

float AView::getTotalFieldVertical() const
{
    return mPadding.vertical() + mMargin.vertical();
}

int AView::getContentMinimumWidth()
{
    return 0;
}

int AView::getContentMinimumHeight()
{
    return 0;
}

bool AView::hasFocus() const
{
    return mHasFocus;
}

int AView::getMinimumWidth()
{
    ensureAssUpdated();
    return (mFixedSize.x == 0 ? ((glm::max)(getContentMinimumWidth(), mMinSize.x) + mPadding.horizontal()) : mFixedSize.x);
}

int AView::getMinimumHeight()
{
    ensureAssUpdated();
    return (mFixedSize.y == 0 ? ((glm::max)(getContentMinimumHeight(), mMinSize.y) + mPadding.vertical()) : mFixedSize.y);
}

void AView::getTransform(glm::mat4& transform) const
{
    transform = glm::translate(transform, glm::vec3{ getPosition(), 0.f });
}

AFontStyle& AView::getFontStyle()
{
    return mFontStyle;
}


void AView::pack()
{
    setSize(getMinimumWidth(), getMinimumHeight());
}

const ADeque<AString>& AView::getCssNames() const
{
    return mAssNames;
}

void AView::addAssName(const AString& assName)
{
    mAssNames << assName;
    assert(("empty ass name" && !assName.empty()));
    mAssHelper = nullptr;
}
void AView::removeAssName(const AString& assName)
{
    mAssNames.removeFirst(assName);
    assert(("empty ass name" && !assName.empty()));
    mAssHelper = nullptr;
}

void AView::ensureAssUpdated()
{
    if (mAssHelper == nullptr)
    {
        mAssHelper = _new<AAssHelper>();
        connect(customCssPropertyChanged, mAssHelper,
                &AAssHelper::onInvalidateStateAss);
        connect(mAssHelper->invalidateFullAss, this, [&]()
        {
            mAssHelper = nullptr;
        });
        connect(mAssHelper->invalidateStateAss, me::updateAssState);

        recompileAss();
    }
}

void AView::onMouseEnter()
{
    if (AWindow::shouldDisplayHoverAnimations()) {
        mHovered.set(this, true);
    }
}


void AView::onMouseMove(glm::ivec2 pos)
{
}

void AView::onMouseLeave()
{
    if (AWindow::shouldDisplayHoverAnimations()) {
        mHovered.set(this, false);
    }
}


void AView::onMousePressed(glm::ivec2 pos, AInput::Key button)
{
    mPressed.set(this, true);

    /**
     * If button is pressed on this view, we want to know when the mouse will be released even if mouse outside
     * this view and even the mouse outside the window so we can guarantee that if we got a mouse press event, we will
     * get a mouse release event too.
     */
    if (auto w = AWindow::current())
    {
        if (w != this) {
            connect(w->mouseReleased, this, [&]()
                {
                    auto selfHolder = sharedPtr();
                    AThread::current()->enqueue([&, selfHolder = std::move(selfHolder)]()
                        {
                            // to be sure that isPressed will be false.
                            if (mPressed) {
                                onMouseReleased(pos, button);
                            }
                        });
                    disconnect();
                });
        }
    }
}

void AView::onMouseReleased(glm::ivec2 pos, AInput::Key button)
{
    mPressed.set(this, false);
    emit clickedButton(button);
    switch (button)
    {
        case AInput::LButton:
            emit clicked();
            break;
        case AInput::RButton:
            emit clickedRight();
            break;
    }
}

void AView::onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button)
{
    emit doubleClicked(button);
}

void AView::onMouseWheel(const glm::ivec2& pos, const glm::ivec2& delta) {

}

void AView::onKeyDown(AInput::Key key)
{
    if (key == AInput::Tab) {
        AWindow::current()->focusNextView();
    }
}

void AView::onKeyRepeat(AInput::Key key)
{
}

void AView::onKeyUp(AInput::Key key)
{
}

void AView::onFocusAcquired()
{
    mHasFocus.set(this, true);
}

void AView::onFocusLost()
{
    mHasFocus.set(this, false);
}

void AView::onCharEntered(wchar_t c)
{

}

bool AView::handlesNonMouseNavigation() {
    return false;
}

void AView::setEnabled(bool enabled)
{
    mDirectlyEnabled = enabled;
    updateEnableState();
}
void AView::updateEnableState()
{
    mEnabled.set(this, mDirectlyEnabled && mParentEnabled);
    emit customCssPropertyChanged();
    setSignalsEnabled(mEnabled);
    emit customCssPropertyChanged();
    redraw();
}

void AView::setAnimator(const _<AAnimator>& animator) {
    mAnimator = animator;
    if (mAnimator)
        mAnimator->setView(this);
}

glm::ivec2 AView::getPositionInWindow() const {
    glm::ivec2 p(0);
    for (const AView* i = this; i && i->getParent(); i = i->getParent()) {
        p += i->getPosition();
    }
    return p;
}


void AView::setPosition(const glm::ivec2& position) {
    mPosition = position;
}
void AView::setSize(int width, int height)
{
    /*
    int minWidth = getContentMinimumWidth();
    int minHeight = getContentMinimumHeight();

    // some bias is allowed
    assert(minWidth <= width + 5);
    assert(minHeight <= height + 5);
*/
    if (mFixedSize.x != 0)
    {
        mSize.x = mFixedSize.x;
    }
    else
    {
        mSize.x = width;
        if (mMinSize.x != 0)
            mSize.x = glm::max(mMinSize.x, mSize.x);
    }
    if (mFixedSize.y != 0)
    {
        mSize.y = mFixedSize.y;
    }
    else
    {
        mSize.y = height;
        if (mMinSize.y != 0)
            mSize.y = glm::max(mMinSize.y, mSize.y);
    }
    mSize = glm::min(mSize, mMaxSize);
}

void AView::setGeometry(int x, int y, int width, int height) {
    setPosition({ x, y });
    setSize(width, height);
}

bool AView::consumesClick(const glm::ivec2& pos) {
    return true;
}

void AView::focus() {
    // holding reference here
    auto mySharedPtr = sharedPtr();
    auto window = AWindow::current();

    try {
        auto windowSharedPtr = window->sharedPtr(); // may throw bad_weak

        ui_threadX [window, mySharedPtr = std::move(mySharedPtr), windowSharedPtr = std::move(windowSharedPtr)]() {
            window->setFocusedView(mySharedPtr);
        };
    } catch (...) {
        window->setFocusedView(mySharedPtr);
    }
}

Visibility AView::getVisibilityRecursive() const {
    if (mVisibility == Visibility::GONE)
        return Visibility::GONE;

    int v = int(mVisibility);

    for (auto target = mParent; target; target = target->mParent) {
        if (v < int(target->mVisibility)) {
            v = int(target->mVisibility);
            if (v == int(Visibility::GONE)) {
                return Visibility::GONE;
            }
        }
    }

    return static_cast<Visibility>(v);
}

void AView::onDpiChanged() {
    mAssHelper = nullptr;
}

void AView::invalidateFont() {

}
void AView::notifyParentEnabledStateChanged(bool enabled) {
    mParentEnabled = enabled;
    updateEnableState();
}

bool AView::onGesture(const glm::ivec2& origin, const AGestureEvent& event) {
    return transformGestureEventsToDesktop(origin, event);
}
bool AView::transformGestureEventsToDesktop(const glm::ivec2& origin, const AGestureEvent& event) {
    return std::visit(aui::lambda_overloaded {
        [&](const AFingerDragEvent& e) {
            onMouseWheel(origin, e.delta);
            return true;
        },
        [&](const ALongPressEvent& e) {
            onMousePressed(origin, AInput::RButton);
            onMouseReleased(origin, AInput::RButton);
            return true;
        },
        [&](const auto& e) {
            return false;
        }
    }, event);
}

void AView::applyAssRule(const RuleWithoutSelector& rule) {
    for (auto& d : rule.getDeclarations()) {
        auto slot = d->getDeclarationSlot();
        if (slot != ass::decl::DeclarationSlot::NONE) {
            mAss[int(slot)] = d->isNone() ? nullptr : d;
        }
        d->applyFor(this);
    }
}
