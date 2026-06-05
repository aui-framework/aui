/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "AView.h"
#include "AUI/Common/AException.h"
#include "AUI/Common/IStringable.h"
#include "AUI/Enum/Visibility.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Util/ATokenizer.h"
#include "AUI/Platform/AWindow.h"
#include <AUI/Platform/IRenderingContext.h>
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Render/ADisplayList.h>
#include <AUI/Render/ADisplayListCanvas.hpp>
#include <AUI/Render/RendererCanvas.h>
#include <AUI/Render/IRendererBackend.h>
#include <AUI/Render/ITexture.h>
#include <AUI/Url/AUrl.h>
#include "AUI/Render/RenderHints.h"
#include "AUI/Animator/AAnimator.h"

#include <exception>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <AUI/IO/AStringStream.h>
#include <AUI/Util/kAUI.h>
#include <AUI/ASS/AStylesheet.h>
#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Traits/callables.h>
#include <AUI/Traits/iterators.h>
#include <stack>
#include <AUI/Action/AMenu.h>

#include "AUI/Platform/ADesktop.h"
#include "AUI/Platform/AFontManager.h"
#include "AUI/Util/AMetric.h"
#include "AUI/Util/Factory.h"
#include "ALabel.h"

// windows.h
#undef max
#undef min

static constexpr auto DEFINITELY_INVALID_SIZE = std::numeric_limits<int>::min() / 2;

ASurface* AView::getWindow() const
{

    AView* parent = nullptr;

    for (AView* target = const_cast<AView*>(this); target; target = target->mParent) {
        parent = target;
    }

    return dynamic_cast<ASurface*>(parent);
}

AView::AView()
{
    AUI_ASSERT_UI_THREAD_ONLY()
    aui::zero(mAss);
    setSlotsCallsOnlyOnMyThread(true);
}

AView::~AView() {
    AUI_ASSERT_UI_THREAD_ONLY();
}

void AView::redraw()
{
    AUI_ASSERT_UI_THREAD_ONLY();
    if (mRedrawRequested) {
        return;
    }
    static constexpr auto EXTRA_OFFSET = 8;
    auto invalidRect = ARect<int>::fromTopLeftPositionAndSize(glm::ivec2(-EXTRA_OFFSET), getSize() + glm::ivec2(EXTRA_OFFSET * 2));
    for (auto s : mAss) {
        AUI_NULLSAFE(s)->updateInvalidPixelRect(invalidRect);
    }
    markPixelDataInvalid(invalidRect);
    mRedrawRequested = true;
}
void AView::markMinContentSizeInvalid()
{
    AUI_ASSERT_UI_THREAD_ONLY();
    mCachedMinContentSize.reset();
    if (mMarkedMinContentSizeInvalid) {
        // already marked.
        // TODO uncomment this
//        for (auto i = getParent(); i; i = i->getParent()) {
//            AUI_ASSERT(i->mMarkedMinContentSizeInvalid);
//        }
        // return;
    }
    mMarkedMinContentSizeInvalid = true;
    AUI_NULLSAFE(mParent)->markMinContentSizeInvalid();
}

void AView::postRender(ARenderContext ctx) {
    if (mAnimator) {
        mAnimator->postRender(this, ctx.canvas);
    }
    emit redrawn;
}

void AView::render(ARenderContext ctx) {
    if (mAnimator) {
        mAnimator->animate(this, ctx.canvas);
    }

    ensureAssUpdated();

    // draw list
    for (unsigned i = 0; i <= int(ass::prop::PropertySlot::SHADOW); ++i) {
        if (i == int(ass::prop::PropertySlot::BACKGROUND_EFFECT)) continue;
        if (auto w = mAss[i]) {
            w->renderFor(this, ctx);
        }
    }

    // mask
    if (mOverflow == AOverflow::HIDDEN_FROM_THIS || mOverflow == AOverflow::HIDDEN) {
        if (mOverflowMask == AOverflowMask::ROUNDED_RECT) {
            ctx.canvas.pushClipRoundedRect(ARect<float>::fromTopLeftPositionAndSize({0, 0}, getSize()), getBorderRadius().getValuePx());
        } else if (mOverflowMask == AOverflowMask::RECT) {
            ctx.canvas.pushClipRect(ARect<float>::fromTopLeftPositionAndSize({0, 0}, getSize()));
        } else {
            if (!mMaskTexture || mMaskTexture->getSize() != glm::u32vec2(getSize())) {
                mMaskTexture = ctx.backend.createTexture(getSize(), APixelFormat::R8_UNORM);
                mRedrawRequested = true;
            }
            if (mRedrawRequested) {
                auto pass = ctx.backend.beginOffscreen(mMaskTexture);
                auto offctx = pass->context();
                offctx.canvas.clear();
                APaint maskPaint;
                maskPaint.brush = ASolidBrush{AColor::WHITE};
                offctx.canvas.roundedRectangle(maskPaint, {0, 0}, getSize(), getBorderRadius());
                ctx.backend.endOffscreen(std::move(pass));
                mRedrawRequested = false;
            }
            ctx.canvas.pushMask(mMaskTexture, glm::vec4(0.f, 0.f, getSize()));
        }
    }

    // draw list
    for (unsigned i = unsigned(ass::prop::PropertySlot::SHADOW) + 1u; i < unsigned(ass::prop::PropertySlot::COUNT);
         ++i) {
        if (i == int(ass::prop::PropertySlot::BACKGROUND_EFFECT))
            continue;
        if (auto w = mAss[i]) {
            w->renderFor(this, ctx);
        }
    }

    if (auto w = mAss[int(ass::prop::PropertySlot::BACKGROUND_EFFECT)]) {
        w->renderFor(this, ctx);
    }
    mRedrawRequested = false;
}

static void walkToParentStack(AView* view, aui::invocable<AView*> auto&& callback) {
    if (view == nullptr) {
        return;
    }
    walkToParentStack(view->getParent(), callback);
    callback(view);
}

void AView::invalidateAllStyles()
{
    auto prevMinSize = mCachedMinContentSize ? getMinimumSizePlusMargin() : glm::ivec2(DEFINITELY_INVALID_SIZE);
    AUI_ASSERTX(mAssHelper != nullptr, "invalidateAllStyles requires mAssHelper to be initialized");

    auto collectRules = [this](const AStylesheet& sh) {
        for (const auto& r : sh.getRules()) {
            if (r.getSelector().isPossiblyApplicable(this)) {
                mAssHelper->mPossiblyApplicableRules << r;
                r.getSelector().setupConnections(this, mAssHelper);
            }
        }
    };

    collectRules(AStylesheet::global());

    walkToParentStack(this, [&](AView* v) {
        if (!v->mExtraStylesheet) {
            return;
        }
        collectRules(*v->mExtraStylesheet);
    });

    invalidateStateStylesImpl(prevMinSize);
}

void AView::invalidateStateStylesImpl(glm::ivec2 prevMinimumSizePlusField) {
    if (!mAssHelper) return;
    mCursor.reset();
    mOverflow = AOverflow::VISIBLE;
    mMargin = {};
    mMinSize = {};
    mBorderRadius = 0;
    //mForceStencilForBackground = false;
    mMaxSize = glm::ivec2(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    mOpacity = 1;
    mTextColor = AColor::BLACK;
    aui::zero(mAss);
    mAssHelper->state.backgroundCropping.size.reset();
    mAssHelper->state.backgroundCropping.offset.reset();
    mAssHelper->state.backgroundImage.reset();
    mAssHelper->state.backgroundUrl.dpiMargin.reset();
    mAssHelper->state.backgroundUrl.overlayColor.reset();
    mAssHelper->state.backgroundUrl.rep.reset();
    mAssHelper->state.backgroundUrl.scale.reset();
    mAssHelper->state.backgroundUrl.sizing.reset();
    mAssHelper->state.backgroundUrl.image.reset();

    for (const auto& r : mAssHelper->mPossiblyApplicableRules) {
        if (r.getSelector().isStateApplicable(this)) {
            applyAssRule(r);
        }
    }
    applyAssRule(mCustomStyleRule);
    commitStyle();

    if (prevMinimumSizePlusField != getMinimumSizePlusMargin()) {
        mMarkedMinContentSizeInvalid = true;
        AUI_NULLSAFE(mParent)->markMinContentSizeInvalid();
    }
    redraw();
}

int AView::getContentMinimumWidth() {
    return 0;
}

int AView::getContentMinimumHeight() {
    return 0;
}

bool AView::hasFocus() const
{
    return mHasFocus;
}

int AView::getMinimumWidth() {
    ensureAssUpdated();
    return (mFixedSize.x == 0 ? ((glm::clamp)(getContentMinimumSize().x + mPadding.horizontal(), mMinSize.x, mMaxSize.x)) : mFixedSize.x);
}

int AView::getMinimumHeight() {
    ensureAssUpdated();
    return (mFixedSize.y == 0 ? ((glm::clamp)(getContentMinimumSize().y + mPadding.vertical(), mMinSize.y, mMaxSize.y)) : mFixedSize.y);
}

void AView::getTransform(glm::mat4& transform) const
{
    transform = glm::translate(transform, glm::vec3{ getPosition(), 0.f });
}

void AView::pack()
{
    setSize({ getMinimumWidth(), getMinimumHeight() });
}

void AView::addAssName(const AString& assName)
{
    AUI_ASSERTX(!assName.empty(), "empty ass name");
    if (mAssNames.contains(assName)) {
        return;
    }
    mAssNames << assName;
    invalidateAssHelper();
}

void AView::invalidateAssHelper() {
    mAssHelper = nullptr;
    aui::zero(mAss);
    redraw();
}

void AView::removeAssName(const AString& assName)
{
    AUI_ASSERTX(!assName.empty(), "empty ass name");
    mAssNames.removeAll(assName);
    invalidateAssHelper();
}


namespace {
    void setupConnectionsCustomStyleRecursive(AView* view, const ass::PropertyListRecursive& propertyList) {
        for (const auto& d : propertyList.conditionalPropertyLists()) {
            d.selector.setupConnections(view, view->getAssHelper());
            setupConnectionsCustomStyleRecursive(view, d.list);
        }
    }
}

void AView::ensureAssUpdated()
{
    if (mAssHelper == nullptr)
    {
        mAssHelper = _new<AAssHelper>();

        setupConnectionsCustomStyleRecursive(this, mCustomStyleRule);

        connect(customCssPropertyChanged, mAssHelper,
                &AAssHelper::onInvalidateStateAss);
        connect(mAssHelper->invalidateFullAss, this, [&]()
        {
            mAssHelper = nullptr;
        });
        connect(mAssHelper->invalidateStateAss, me::invalidateStateStyles);

        invalidateAllStyles();
    }
}

void AView::onMouseEnter()
{
    mMouseEntered = true;
    if (AWindow::current()->shouldDisplayHoverAnimations()) {
        mHovered.set(this, true);
    }
}


void AView::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event)
{
    if (mCursor) {
        AUI_NULLSAFE(AWindow::current())->setCursor(mCursor);
    }
}

void AView::onMouseLeave()
{
    mMouseEntered = false;
    mHovered.set(this, false);
}


void AView::onPointerPressed(const APointerPressedEvent& event)
{
    if (!mPressed.contains(event.pointerIndex)) {
        mPressed << event.pointerIndex;
        emit pressedState(true, event.pointerIndex);
        emit pressed(event.pointerIndex);
    }
}

void AView::onPointerReleased(const APointerReleasedEvent& event)
{
    mPressed.removeAll(event.pointerIndex);
    try {
        emit pressedState(false, event.pointerIndex);
        emit released(event.pointerIndex);

        if (event.triggerClick) {
            emit clickedButton(event.pointerIndex);
            if (event.asButton == AInput::LBUTTON) {
                emit clicked();
            }
            switch (event.pointerIndex.rawValue()) {
                case AInput::RBUTTON:
                    emit clickedRight;
                    emit clickedRightOrLongPressed;

                    auto menuModel = composeContextMenu();
                    if (!menuModel.empty()) {
                        AMenu::show(menuModel);
                    }

                    break;
            }
        }
    } catch (const AException& e) {
        ALogger::err("AView") << "Unhandled exception in clicked signal: " << e;
    } catch (const std::exception& e) {
        ALogger::err("AView") << "Unhandled exception in clicked signal: " << e;
    }
}

AMenuModel AView::composeContextMenu() {
    return {};
}

void AView::onPointerDoubleClicked(const APointerPressedEvent& event)
{
    emit doubleClicked(event.pointerIndex);
}

void AView::onScroll(const AScrollEvent& event) {
    emit scrolled(event.delta);
}

void AView::onKeyDown(AInput::Key key)
{
    emit keyPressed(key);
}

void AView::onKeyRepeat(AInput::Key key)
{
}

void AView::onKeyUp(AInput::Key key)
{
    emit keyReleased(key);
}

void AView::onFocusAcquired()
{
    mHasFocus.set(this, true);
}

void AView::onFocusLost()
{
    mHasFocus.set(this, false);
}

void AView::onCharEntered(AChar c)
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
    if (!mEnabled) {
        onMouseLeave();
    }

    bool newEnabled = mDirectlyEnabled && mParentEnabled;
    if (mEnabled == newEnabled) {
        return;
    }

    mEnabled = newEnabled;
    emit mEnabledChanged(newEnabled);
    emit customCssPropertyChanged;
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


void AView::setPosition(glm::ivec2 position) {
    mSkipUntilLayoutUpdate = false;
    if (mPosition == position) [[unlikely]] {
        return;
    }
    mPosition = position;
    redraw();
    emit mPositionChanged(position);
}
void AView::setSize(glm::ivec2 size)
{
    mMarkedMinContentSizeInvalid = false;
    mSkipUntilLayoutUpdate = false;
    auto newSize = mSize;
    if (mFixedSize.x != 0)
    {
        newSize.x = mFixedSize.x;
    }
    else
    {
        newSize.x = size.x;
        if (mMinSize.x != 0)
            newSize.x = glm::max(mMinSize.x, newSize.x);
    }
    if (mFixedSize.y != 0)
    {
        newSize.y = mFixedSize.y;
    }
    else
    {
        newSize.y = size.y;
        if (mMinSize.y != 0)
            newSize.y = glm::max(mMinSize.y, newSize.y);
    }
    newSize = glm::min(newSize, mMaxSize);

    if (mSize == newSize) [[unlikely]] {
        return;
    }
    mSize = newSize;
    redraw();
    emit mSizeChanged(newSize);
}

void AView::setGeometry(int x, int y, int width, int height) {
    mSkipUntilLayoutUpdate = false;
    auto oldPosition = mPosition;
    auto oldSize = mSize;
    setPosition({ x, y });
    setSize({width, height});

    if (mPosition == oldPosition && mSize == oldSize) [[unlikely]] {
        return;
    }
    emit geometryChanged({x, y}, {width, height});
}

bool AView::consumesClick(const glm::ivec2& pos) {
    if (mSkipUntilLayoutUpdate) {
        return false;
    }
    return true;
}

void AView::notifyParentChildFocused(const _<AView>& view) {
    if (mParent == nullptr)
        return;

    emit mParent->childFocused(view);
    mParent->notifyParentChildFocused(view);
}

void AView::focus(bool needFocusChainUpdate) {
    // holding reference here
    auto mySharedPtr = aui::ptr::shared_from_this(this);

    notifyParentChildFocused(mySharedPtr);

    AUI_UI_THREAD_X [mySharedPtr = std::move(mySharedPtr), needFocusChainUpdate]() {
        auto window = mySharedPtr->getWindow();
        if (!window) {
            return;
        }
        window->setFocusedView(mySharedPtr);
        if (needFocusChainUpdate) {
            window->updateFocusChain();
        }
    };
}

bool AView::capturesFocus() {
    return true;
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

void AView::notifyParentEnabledStateChanged(bool enabled) {
    mParentEnabled = enabled;
    updateEnableState();
}

bool AView::onGesture(const glm::ivec2& origin, const AGestureEvent& event) {
    return transformGestureEventsToDesktop(origin, event);
}
bool AView::transformGestureEventsToDesktop(const glm::ivec2& origin, const AGestureEvent& event) {
    return std::visit(aui::lambda_overloaded {
        [&](const ALongPressEvent& e) {
            auto menuModel = composeContextMenu();
            bool result = false;
            if (clickedRightOrLongPressed) {
                emit clickedRightOrLongPressed;
                result = true;
            }
            if (!menuModel.empty()) {
                AMenu::show(menuModel);
                result = true;
            }
            return result;
        },
        [&](const auto& e) {
            return false;
        }
    }, event);
}

void AView::applyAssRule(const ass::PropertyList& propertyList) {
    for (const auto& d : propertyList.declarations()) {
        auto slot = d->getPropertySlot();
        if (slot != ass::prop::PropertySlot::NONE) {
            mAss[int(slot)] = d->isNone() ? nullptr : d.get();
        }
        d->applyFor(this);
    }
}

void AView::applyAssRule(const ass::PropertyListRecursive& propertyList) {
    applyAssRule(static_cast<const ass::PropertyList&>(propertyList));

    for (const auto& d : propertyList.conditionalPropertyLists()) {
        if (d.selector.isStateApplicable(this)) {
            applyAssRule(d.list);
        }
    }
}

ALayoutDirection AView::parentLayoutDirection() const noexcept {
    if (mParent == nullptr) return ALayoutDirection::NONE;
    if (mParent->getLayout() == nullptr) return ALayoutDirection::NONE;
    return mParent->getLayout()->getLayoutDirection();
}


void AView::setCustomStyle(ass::PropertyListRecursive rule) {
    AUI_ASSERT_UI_THREAD_ONLY();
    mCustomStyleRule = std::move(rule);
    if (mAssHelper == nullptr) {
        return;
    }
    auto prevMinSize = mCachedMinContentSize ? getMinimumSizePlusMargin() : glm::ivec2(DEFINITELY_INVALID_SIZE);
    AUI_ASSERTX(mAssHelper != nullptr, "invalidateAllStyles requires mAssHelper to be initialized");
    invalidateStateStylesImpl(prevMinSize);
}


bool AView::hasIndirectParent(const _<AView>& v) {
    for (auto p = getParent(); p != nullptr; p = p->getParent()) {
        if (p == v.get()) {
            return true;
        }
    }
    return false;
}

bool AView::wantsTouchscreenKeyboard() {
    return false;
}

void AView::setExtraStylesheet(AStylesheet&& extraStylesheet) {
    mExtraStylesheet = _new<AStylesheet>(std::move(extraStylesheet));
    invalidateAssHelper();
}

void AView::onClickPrevented() {
    auto pressed = std::move(mPressed);
    for (auto v : pressed) {
        emit pressedState(false, v);
        emit released(v);
    }
}

void AView::setCursor(AOptional<ACursor> cursor) {
    mCursor = std::move(cursor);
    if (mParent) { // ASurface does not have parent
        AWindow::current()->forceUpdateCursor();
    }
}

void AView::onViewGraphSubtreeChanged() {
    invalidateAssHelper();
    emit viewGraphSubtreeChanged;
}
void AView::setVisibility(Visibility visibility) noexcept
{
    if (mVisibility == visibility) {
        return;
    }
    auto prev = std::exchange(mVisibility, visibility);
    if ((mVisibility & Visibility::FLAG_CONSUME_SPACE) != (prev & Visibility::FLAG_CONSUME_SPACE)) {
        mMarkedMinContentSizeInvalid = false; // force
        markMinContentSizeInvalid();
    }
    redraw();
    emit mVisibilityChanged(visibility);
}

namespace aui::view::impl {
    bool isDefinitelyInvisible(AView& view);
}

void AView::markPixelDataInvalid(ARect<int> invalidArea) {
    if (getOverflow() != AOverflow::VISIBLE) {
        // clip by overflow
        invalidArea.p1 = glm::max(invalidArea.p1, glm::ivec2(0));
        invalidArea.p2 = glm::min(invalidArea.p2, getSize());
    }

    if (mRedrawRequested) {
        // this view already requested a redraw.
        return;
    }

    AUI_NULLSAFE(mParent)->markPixelDataInvalid(invalidArea.translate(getPosition()));
}

AString AView::debugString() const {
    return "{} at {}"_format(mAssNames.empty() ? IStringable::toString(this) : mAssNames.last(), getPositionInWindow());
}

void AView::forceUpdateLayoutRecursively() {
    AUI_DO_ONCE {
        ALogger::warn("AView") << "AView::forceUpdateLayoutRecursively() called; it's for debugging purposes only.";
    }
    mMarkedMinContentSizeInvalid = true;
    mCachedMinContentSize.reset();
}

void AView::commitStyle() {

}

std::ostream& operator<<(std::ostream& os, const AView& view) {
    os << view.debugString();
    return os;
}
