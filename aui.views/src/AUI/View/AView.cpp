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

#include <exception>
#include <limits>
#include <stack>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Common/AException.h>
#include <AUI/Common/IStringable.h>
#include <AUI/Enum/Visibility.h>
#include <AUI/Render/IRenderer.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Url/AUrl.h>
#include <AUI/Render/RenderHints.h>
#include <AUI/Animator/AAnimator.h>
#include <AUI/IO/AStringStream.h>
#include <AUI/Util/kAUI.h>
#include <AUI/ASS/AStylesheet.h>
#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Traits/callables.h>
#include <AUI/Traits/iterators.h>
#include <AUI/Action/AMenu.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/AFontManager.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Util/Factory.h>
#include <AUI/View/ALabel.h>

// windows.h
#undef max
#undef min

ASurface* AView::getWindow() const {
  AView* parent = nullptr;

  for (AView* target = const_cast<AView*>(this); target; target = target->mParent) {
    parent = target;
  }

  return dynamic_cast<ASurface*>(parent);
}

AView::AView() {
  AUI_ASSERT_UI_THREAD_ONLY()
  aui::zero(mAss);
  setSlotsCallsOnlyOnMyThread(true);
}

AView::~AView() {
  AUI_ASSERT_UI_THREAD_ONLY();
}

void AView::redraw() {
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

void AView::requestLayout() {
  AUI_ASSERT_UI_THREAD_ONLY();
  mWantsLayoutUpdate = true;
  mLastLayoutSize = glm::ivec2(-1, -1);
  mMeasureCache.clear();
  mMinMaxSizesCache.clear();
  if (mFixedSize == glm::ivec2(0) && static_cast<bool>(getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
    AUI_NULLSAFE(mParent)->requestLayout();
  }
}

void AView::drawStencilMask(ARenderContext ctx) {
  switch (mOverflowMask) {
    case AOverflowMask::ROUNDED_RECT:
      if (mBorderRadius > 0) {
        ctx.render.roundedRectangle(ASolidBrush{},
                             {mPadding.left, mPadding.top},
                             {getWidth() - mPadding.horizontal(), getHeight() - mPadding.vertical()},
                             glm::max(mBorderRadius - std::min(mPadding.horizontal(), mPadding.vertical()), 0.f));
      } else {
        ctx.render.rectangle(ASolidBrush{},
                             {mPadding.left, mPadding.top},
                             {getWidth() - mPadding.horizontal(), getHeight() - mPadding.vertical()});
      }
      break;

    case AOverflowMask::BACKGROUND_IMAGE_ALPHA:
      if (auto s = mAss[int(ass::prop::PropertySlot::BACKGROUND_IMAGE)]) {
        s->renderFor(this, ctx);
      }
      break;
  }
}

void AView::postRender(ARenderContext ctx) {
  if (mAnimator) {
    mAnimator->postRender(this, ctx.render);
  }
  popStencilIfNeeded(ctx);

  emit redrawn;
}

void AView::popStencilIfNeeded(ARenderContext ctx) {
  if (getOverflow() == AOverflow::HIDDEN || getOverflow() == AOverflow::HIDDEN_FROM_THIS) {
    /*
     * If the AView's Overflow set to Overflow::HIDDEN AView pushed it's mask into the stencil buffer but AView
     * cannot return stencil buffer to the previous state by itself because of C++ restrictions. We should also
     * apply mask AFTER transform updated and BEFORE rendering AView content. The only way to return the stencil
     * back is place it here, after rendering AView.
     */
    RenderHints::popMask(ctx.render, [&] {
      drawStencilMask(ctx);
    });
  }
}

void AView::render(ARenderContext ctx) {
    if (mAnimator)
        mAnimator->animate(this, ctx.render);

    ensureAssUpdated();

    // draw list
    for (unsigned i = 0; i <= int(ass::prop::PropertySlot::SHADOW); ++i) {
        if (i == int(ass::prop::PropertySlot::BACKGROUND_EFFECT)) continue;
        if (auto w = mAss[i]) {
            w->renderFor(this, ctx);
        }
    }

    //draw before drawing this element
    if (mOverflow == AOverflow::HIDDEN_FROM_THIS)
    {
        RenderHints::pushMask(ctx.render, [&] {
            drawStencilMask(ctx);
        });
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

    //draw stencil before drawing children elements
    if (mOverflow == AOverflow::HIDDEN)
    {
        RenderHints::pushMask(ctx.render, [&] {
            drawStencilMask(ctx);
        });
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

    invalidateStateStylesImpl();
}

void AView::invalidateStateStylesImpl() {
    if (!mAssHelper) return;
    mCursor.reset();
    mOverflow = AOverflow::VISIBLE;
    mMargin = {};
    mMinSize = {};
    mBorderRadius = 0.f;
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

    mApplyingStyles = true;
    AUI_DEFER {
        mApplyingStyles = false;
    };

    for (const auto& r : mAssHelper->mPossiblyApplicableRules) {
        if (r.getSelector().isStateApplicable(this)) {
            applyAssRule(r);
        }
    }
    applyAssRule(mCustomStyleRule);
    if (mExplicitMinSize) {
        mMinSize = *mExplicitMinSize;
    }
    commitStyle();

    requestLayout();
    redraw();
}

bool AView::hasFocus() const {
    return mHasFocus;
}

AMinMaxAxis AView::computeMinMaxAxis(int height) {
  ensureAssUpdated();
  if (auto it = mMinMaxSizesCache.find(height); it != mMinMaxSizesCache.end()) {
    return it->second;
  }

  auto sizes = onComputeIntrinsicMinMaxAxis(height == -1 ? -1 : std::max(0, height - mPadding.vertical()));
  sizes.min += mPadding.horizontal();
  sizes.max += mPadding.horizontal();

  if (mFixedSize.x != 0) {
    sizes.min = sizes.max = mFixedSize.x;
  } else {
    sizes.min = std::max(sizes.min, mMinSize.x);
    sizes.max = std::max(sizes.max, sizes.min);
    if (mMaxSize.x != -1) {
      sizes.max = std::min(sizes.max, mMaxSize.x);
      sizes.min = std::min(sizes.min, sizes.max);
    }
  }

  return mMinMaxSizesCache.emplace(height, sizes).first->second;
}

glm::ivec2 AView::measure(AConstraints constraints) {
  ensureAssUpdated();
  if (auto it = mMeasureCache.find(constraints); it != mMeasureCache.end()) {
    return it->second;
  }

  AConstraints effective = constraints;
  const bool requestedUnlimitedInline = effective.maxInline == -1;
  const bool requestedUnlimitedBlock = effective.maxBlock == -1;
  const bool passUnlimitedInline = requestedUnlimitedInline && mFixedSize.x == 0;
  const bool passUnlimitedBlock = requestedUnlimitedBlock && mFixedSize.y == 0;
  bool unlimitedInline = requestedUnlimitedInline;
  bool unlimitedBlock = requestedUnlimitedBlock;

  if (mFixedSize.x != 0) {
    effective.minInline = effective.maxInline = mFixedSize.x;
    unlimitedInline = false;
  } else {
    effective.minInline = std::max(effective.minInline, mMinSize.x);
    if (effective.minInline == 0 && effective.maxInline == 0) {
      effective.minInline = computeMinMaxAxis(passUnlimitedBlock ? -1 : effective.maxBlock).min;
    }
    if (mMaxSize.x != -1) {
      effective.maxInline = unlimitedInline ? mMaxSize.x : std::min(effective.maxInline, mMaxSize.x);
      unlimitedInline = false;
    }
  }
  if (mFixedSize.y != 0) {
    effective.minBlock = effective.maxBlock = mFixedSize.y;
    unlimitedBlock = false;
  } else {
    effective.minBlock = std::max(effective.minBlock, mMinSize.y);
    if (mMaxSize.y != -1) {
      effective.maxBlock = unlimitedBlock ? mMaxSize.y : std::min(effective.maxBlock, mMaxSize.y);
      unlimitedBlock = false;
    }
  }

  const int effectiveMaxInline = unlimitedInline
      ? std::numeric_limits<int>::max()
      : std::max(effective.minInline, effective.maxInline);
  const int effectiveMaxBlock = unlimitedBlock
      ? std::numeric_limits<int>::max()
      : std::max(effective.minBlock, effective.maxBlock);

  int hPadding = mPadding.left + mPadding.right;
  int vPadding = mPadding.top + mPadding.bottom;

  AConstraints content;
  content.minInline = std::max(0, effective.minInline - hPadding);
  content.maxInline = passUnlimitedInline ? -1 : (unlimitedInline ? -1 : std::max(0, effectiveMaxInline - hPadding));
  content.minBlock = std::max(0, effective.minBlock - vPadding);
  content.maxBlock = passUnlimitedBlock ? -1 : (unlimitedBlock ? -1 : std::max(0, effectiveMaxBlock - vPadding));

  glm::ivec2 content_size = onIntrinsicMeasure(content);

  glm::ivec2 measured;
  measured.x = std::clamp(content_size.x + hPadding, effective.minInline,  effectiveMaxInline);
  measured.y = std::clamp(content_size.y + vPadding, effective.minBlock, effectiveMaxBlock);

  return mMeasureCache.emplace(constraints, measured).first->second;
}

glm::ivec2 AView::onIntrinsicMeasure(AConstraints constraints) {
  const auto minMax =
      onComputeIntrinsicMinMaxAxis(constraints.isUnlimitedBlock() ? -1 : constraints.maxBlock);
  const int maxInline = constraints.isUnlimitedInline() ? std::numeric_limits<int>::max() : constraints.maxInline;
  return {
    std::clamp(minMax.max, constraints.minInline, maxInline),
    constraints.minBlock,
  };
}

AMinMaxAxis AView::onComputeIntrinsicMinMaxAxis(int height) {
  return {};
}

void AView::getTransform(glm::mat4& transform) const {
  transform = glm::translate(transform, glm::vec3{ getPosition(), 0.f });
}

void AView::pack() {
  layout(getPosition(), measure(AConstraints {}));
}

void AView::addAssName(const AString& assName) {
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

void AView::removeAssName(const AString& assName) {
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

void AView::ensureAssUpdated() {
  if (mAssHelper == nullptr) {
    mAssHelper = _new<AAssHelper>();

    setupConnectionsCustomStyleRecursive(this, mCustomStyleRule);

    connect(customCssPropertyChanged, mAssHelper,
            &AAssHelper::onInvalidateStateAss);
    connect(mAssHelper->invalidateFullAss, this, [&]() {
      mAssHelper = nullptr;
    });
    connect(mAssHelper->invalidateStateAss, me::invalidateStateStyles);

    invalidateAllStyles();
  }
}

void AView::onMouseEnter() {
  mMouseEntered = true;
  if (ASurface::current()->shouldDisplayHoverAnimations()) {
    mHovered.set(this, true);
  }
}

void AView::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
  if (mCursor) {
    AUI_NULLSAFE(ASurface::current())->setCursor(mCursor);
  }
}

void AView::onMouseLeave() {
  mMouseEntered = false;
  mHovered.set(this, false);
}

void AView::onPointerPressed(const APointerPressedEvent& event) {
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
    if (mPosition == position) [[unlikely]] {
        return;
    }
    mPosition = position;
    redraw();
    emit mPositionChanged(position);
}

void AView::setSize(glm::ivec2 size) {
  auto newSize = mSize;
  if (mFixedSize.x != 0) {
    newSize.x = mFixedSize.x;
  } else {
    newSize.x = size.x;
    if (mMinSize.x != 0) {
      newSize.x = glm::max(mMinSize.x, newSize.x);
    }
  }
  if (mFixedSize.y != 0) {
    newSize.y = mFixedSize.y;
  } else {
    newSize.y = size.y;
    if (mMinSize.y != 0) {
      newSize.y = glm::max(mMinSize.y, newSize.y);
    }
  }
  if (mMaxSize.x != -1) {
    newSize.x = glm::min(newSize.x, mMaxSize.x);
  }
  if (mMaxSize.y != -1) {
    newSize.y = glm::min(newSize.y, mMaxSize.y);
  }

  if (mSize == newSize) [[unlikely]] {
    return;
  }
  mSize = newSize;
  redraw();
  emit mSizeChanged(newSize);
}

void AView::layout(int x, int y, int w, int h) {
  if (mPosition == glm::ivec2(x, y) && mLastLayoutSize == glm::ivec2(w, h) && !mWantsLayoutUpdate) {
    return;
  }
  auto oldPosition = mPosition;
  auto oldSize = mSize;
  setPosition({ x, y });
  setSize({ w, h });
  onLayout(mSize.x, mSize.y);
  mSkipUntilLayoutUpdate = false;
  mWantsLayoutUpdate = false;
  mLastLayoutSize = mSize;
  if (oldPosition != mPosition || oldSize != mSize) {
    emit geometryChanged(mPosition, mSize);
  }
}

void AView::onLayout(int w, int h) {
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
    AUI_ASSERTX(mAssHelper != nullptr, "invalidateAllStyles requires mAssHelper to be initialized");
    invalidateStateStylesImpl();
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
        ASurface::current()->forceUpdateCursor();
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
        requestLayout();
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
    if (mRenderToTexture) {
        if (glm::all(glm::lessThanEqual(invalidArea.p1, glm::ivec2(0, 0))) &&
            glm::all(glm::greaterThanEqual(invalidArea.p2, getSize()))) {
            mRenderToTexture->invalidArea = IRenderViewToTexture::InvalidArea::Full{};
        }
        mRenderToTexture->invalidArea.addRectangle(invalidArea);
        if (std::exchange(mRedrawRequested, true)) {
            // this view already requested a redraw.
            return;
        }
        // temporary disable drawing from texture. this will be set back to true by the callback below.
        mRenderToTexture->drawFromTexture = false;
        ASurface::current()->beforeFrameQueue().enqueue([this, self = aui::ptr::shared_from_this(this)](IRenderer& renderer) {
            if (!mRenderToTexture || !mRenderToTexture->rendererInterface) {
                // dead interface?
                return;
            }

            if (mRenderToTexture->skipRedrawUntilTextureIsPresented) {
                // last frame we draw here was not used.
                // we might want to skip drawing a new frame until AViewContainer::drawView flags that the rasterization
                // results are actually displayed.
                mRedrawRequested = false;
                return;
            }

            if (glm::any(glm::equal(getSize(), glm::ivec2(0)))) {
                // unable to render to zero-area texture
                mRedrawRequested = false;
                mRenderToTexture->invalidArea = IRenderViewToTexture::InvalidArea::Empty{};
                return;
            }

            if (mRenderToTexture->invalidArea.empty()) {
                // if we weren't check, begin would throw assertion failed. Theoretically, that should not happen.
                // but why not safe check?
                return;
            }

            APerformanceSection s("Render-to-texture rasterization", {}, debugString().toStdString());
            auto invalidArea = std::exchange(mRenderToTexture->invalidArea, IRenderViewToTexture::InvalidArea::Empty{});
            if (!mRenderToTexture->rendererInterface->begin(renderer, size(), invalidArea)) {
                // unsuccessful
                mRenderToTexture->skipRedrawUntilTextureIsPresented = true;
                return;
            }
            AUI_DEFER {
                mRenderToTexture->rendererInterface->end(renderer);
            };

            ARenderContext contextOfTheView {
                .clippingRects = invalidArea.rectangles() ? ARenderContext::Rectangles(invalidArea.rectangles()->begin(),
                                                                                       invalidArea.rectangles()->end()) : ARenderContext::Rectangles{},
                .render = renderer,
            };
            ARect<int> initialRect {
                .p1 = { 0, 0 },
                .p2 = size(),
            };
            if (contextOfTheView.clippingRects.empty()) {
                contextOfTheView.clippingRects << initialRect;
            } else {
                contextOfTheView.clip(initialRect);
            }
            RenderHints::PushState state(renderer);
            try {
                render(contextOfTheView);
                postRender(contextOfTheView);
            }
            catch (const AException& e) {
                ALogger::err("AView") << "Unable to render view: " << e;
                return;
            }
            mRenderToTexture->skipRedrawUntilTextureIsPresented = true;
            mRenderToTexture->drawFromTexture = true;
        });
        AUI_NULLSAFE(mParent)->markPixelDataInvalid(ARect<int>::fromTopLeftPositionAndSize(getPosition(), size()));
        return;
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
    requestLayout();
}

void AView::commitStyle() {

}

std::ostream& operator<<(std::ostream& os, const AView& view) {
    os << view.debugString();
    return os;
}
