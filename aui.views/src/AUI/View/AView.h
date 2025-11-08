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

#pragma once

#include <chrono>
#include <functional>
#include <array>

#include <glm/glm.hpp>

#include "AUI/Common/ASmallVector.h"
#include <AUI/ASS/Property/IProperty.h>
#include <AUI/ASS/Property/ScrollbarAppearance.h>
#include "AUI/Common/ABoxFields.h"
#include "AUI/Common/ADeque.h"
#include "AUI/Common/AObject.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/Platform/ACursor.h"
#include "AUI/Platform/AInput.h"
#include "AUI/Reflect/AClass.h"
#include "AUI/Font/AFontStyle.h"
#include "AUI/Util/AFieldSignalEmitter.h"
#include "AUI/Render/ARenderContext.h"
#include "AUI/Util/IBackgroundEffect.h"
#include <AUI/ASS/PropertyListRecursive.h>
#include <AUI/Enum/AOverflow.h>
#include <AUI/Enum/Visibility.h>
#include <AUI/Enum/MouseCollisionPolicy.h>
#include <AUI/Util/ALayoutDirection.h>
#include <AUI/Action/AMenu.h>

#include <AUI/Event/AScrollEvent.h>
#include <AUI/Event/AGestureEvent.h>
#include <AUI/Event/APointerPressedEvent.h>
#include <AUI/Event/APointerReleasedEvent.h>
#include <AUI/Event/APointerMoveEvent.h>
#include <AUI/Render/ITexture.h>
#include <AUI/Render/IRenderViewToTexture.h>
#include <AUI/Enum/AFloat.h>
#include <AUI/Common/AProperty.h>


class AWindow;
class AWindowBase;
class AViewContainerBase;
class AAnimator;
class AAssHelper;
class AStylesheet;

/**
 * @brief Base class of all UI objects.
 *
 * @details
 * A class that describes the minimum unit that can be placed in a container (including a window) that takes up
*  some space on the screen, responds to changes in size, position, moving the cursor, pressing / releasing
*  keys and buttons, mouse wheel, etc...
 *
 * Analogue to Qt's QWidget, Android's View.
 *
 * Every view has a position and size. Also, almost every view has parent, in exception to toplevel view, [AWindow].
 * Parent view is [AViewContainerBase] which is responsible for layout of all views inside.
 *
 * @image html Screenshot_20241212_064400.png Devtools demonstrating view hierarchy.
 *
 * AView by itself does not provide much functionality. There are many implementations of the functionality (so called
 * "UI kit"): [AButton], [ATextField], [ACheckBox]. An implementation view can be a container, e.g.
 * [ANumberPicker], which consists of a text field and buttons.
 *
 * You can implement your view by inheriting from AView and overriding some of its methods. Also, you can define
 * [declarative contract](retained_immediate_ui.md), which means you accept everything your view needs
 * (including data), and return pure AView without subclassing.
 *
 * Every view can be [styled](ass.md). It means that it can have a set of properties which can be used to change
 * background, border, text color, etc.
 *
 * The user interaction is driven by events. You can handle them by overriding respective methods in AView. Also, many
 * events can be handled with signal-slot, see which signals is emitted by AView.
 *
 * @ingroup useful_views
 */
class API_AUI_VIEWS AView: public AObject
{
    friend class AViewContainerBase;
    friend class AViewContainer;
    friend class IRenderViewToTexture;
public:
    AView();
    ~AView() override;


    /**
     * @brief Whether view is enabled (i.e., reacts to user).
     */
    auto enabled() const {
        return APropertyDef {
            this,
            &AView::mEnabled,
            &AView::setEnabled,
            mEnabledChanged,
        };
    }

    /**
     * @brief Top left corner's position relative to top left corner's position of the parent AView.
     */
    auto position() const {
        return APropertyDef {
            this,
            &AView::mPosition,
            &AView::setPosition,
            mPositionChanged,
        };
    }

    /**
     * @brief Size, including content area, border and padding.
     */
    auto size() const {
        return APropertyDef {
            this,
            &AView::mSize,
            &AView::setSize,
            mSizeChanged,
        };
    }

    /**
     * @brief Expansion coefficient. Hints layout manager how much this AView should be extended relative to other
     *        AViews in the same container.
     * @details
     * It does affect expanding environment inside the container. See expanding [layout managers](layout-managers.md)
     * for more info.
     *
     * It does not affect parent's size or parent's expanding property. Use AView::setExpanding() on parent, or
     * `Expanding` variant of declarative container notation (`Vertical::Expanding`, `Horizontal::Expanding`,
     * `Stacked::Expanding`) for such case.
     */
    auto expanding() const {
        return APropertyDef {
            this,
            &AView::mExpanding,
            [](AView& self, glm::ivec2 expanding) { self.setExpanding(expanding); },
            mExpandingChanged,
        };
    }

    /**
     * @brief Visibility value.
     */
    auto visibility() const {
        return APropertyDef {
            this,
            &AView::mVisibility,
            &AView::setVisibility,
            mVisibilityChanged,
        };
    }

    /**
     * @brief Request window manager to redraw this AView.
     */
    void redraw();

    /**
     * @brief Determines window which this AView belongs to.
     * @return window which this AView belongs to. Could be nullptr
     */
    AWindowBase* getWindow() const;

    virtual void drawStencilMask(ARenderContext ctx);

    /**
     * @brief Draws this AView. Noone should call this function except rendering routine.
     * @see AView::drawView
     * @details
     * AView::render is not guaranteed to be called on per-frame basis. Moreover, this method can be called multiple
     * times if render-to-texture caching decides to do so.
     */
    virtual void render(ARenderContext ctx);

    /**
     * @brief Performs post-draw routines of this AView. Noone should call this function except rendering routine.
     * @see AView::drawView
     */
    virtual void postRender(ARenderContext ctx);

    void popStencilIfNeeded(ARenderContext ctx);

    [[nodiscard]]
    const AVector<AString>& getAssNames() const noexcept {
        return mAssNames;
    }

    /**
     * @brief Top left corner's position relative to top left corner's position of the parent AView.
     */
    [[nodiscard]]
    glm::ivec2 getPosition() const noexcept
    {
        return mPosition;
    }

    /**
     * @brief The center point position of the view relatively to top left corner of the window.
     * @details
     * Useful in UI tests:
     * ```cpp
     * mWindow->onPointerMove(mView->getCenterPointInWindow()); // triggers on pointer move over the view through window
     * ```
     */
    [[nodiscard]]
    glm::ivec2 getCenterPointInWindow() const noexcept
    {
        return getPositionInWindow() + *size() / 2;
    }

    /**
     * @brief Size, including content area, border and padding.
     */
    [[nodiscard]]
    glm::ivec2 getSize() const noexcept
    {
        return mSize;
    }

    /**
     * @return minSize (ignoring fixedSize)
     */
    glm::ivec2 getMinSize() const noexcept {
        return mMinSize;
    }

    void setMinSize(glm::ivec2 minSize) noexcept {
        mMinSize = minSize;
    }

    /**
     * @brief Marks this view it requires a layout update.
     * @details
     * See [layout-managers] for more info.
     */
    virtual void markMinContentSizeInvalid();

    /**
     * @see mExtraStylesheet
     */
    void setExtraStylesheet(_<AStylesheet> extraStylesheet) {
        mExtraStylesheet = std::move(extraStylesheet);
        invalidateAssHelper();
    }

    /**
     * @see mExtraStylesheet
     */
    void setExtraStylesheet(AStylesheet&& extraStylesheet);

    /**
     * @see mExtraStylesheet
     */
    [[nodiscard]]
    const _<AStylesheet>& extraStylesheet() const noexcept {
        return mExtraStylesheet;
    }

    /**
     * @brief Determines whether display graphics that go out of the bounds of this AView or not.
     */
    AOverflow getOverflow() const
    {
        return mOverflow;
    }
    void setOverflow(AOverflow overflow)
    {
        mOverflow = overflow;
    }

    /**
     * @brief Controls how does the overflow (stencil) mask is produced.
     */
    AOverflowMask getOverflowMask() const
    {
        return mOverflowMask;
    }
    void setOverflowMask(AOverflowMask overflow)
    {
        mOverflowMask = overflow;
    }

    /**
     * @brief border-radius, specified in ASS.
     */
    float getBorderRadius() const {
        return mBorderRadius;
    }
    void setBorderRadius(float radius) {
        mBorderRadius = radius;
    }

    int getWidth() const
    {
        return mSize.x;
    }

    int getHeight() const
    {
        return mSize.y;
    }

    /**
     * @return pixel count which this AView acquired by width including content area, padding, border and margin.
     */
    int getTotalOccupiedWidth() const
    {
        return !(mVisibility & Visibility::FLAG_CONSUME_SPACE) ? 0 : mSize.x + getTotalFieldHorizontal();
    }

    /**
     * @return pixel count which this AView acquired by height including content area, padding, border and margin.
     */
    int getTotalOccupiedHeight() const
    {
        return !(mVisibility & Visibility::FLAG_CONSUME_SPACE) ? 0 : mSize.y + getTotalFieldVertical();
    }

    /**
     * @return minimum content size plus margin.
     * @details
     * This value is bare minimum space required for this view. It includes minimal content size + padding + margin which
     * is exact space the view requires.
     */
    [[nodiscard]]
    glm::ivec2 getMinimumSizePlusMargin() {
        return getMinimumSize() + mMargin.occupiedSize();
    }

    /**
     * @brief Returns the [margin](AView::mMargin).
     * @return margin
     * @details
     * @copydetails AView::mMargin
     */
    [[nodiscard]]
    const ABoxFields& getMargin()
    {
        ensureAssUpdated();
        return mMargin;
    }

    /**
     * @brief Sets the [margin](AView::mMargin).
     * @param margin margin
     * @details
     * @copydetails AView::mMargin
     */
    void setMargin(const ABoxFields& margin) {
        mMargin = margin;
    }

    /**
     * @brief Determines whether this AView processes this click or passes it thru.
     * @param pos mouse position
     * @return true if AView processes this click
     * @details
     * Used in AViewContainer::getViewAt method subset, thus affecting click event handling.
     */
    virtual bool consumesClick(const glm::ivec2& pos);

    /**
     * @brief Returns the [padding](AView::mPadding).
     * @return padding
     */
    [[nodiscard]]
    const ABoxFields& getPadding()
    {
        ensureAssUpdated();
        return mPadding;
    }

    /**
     * @brief Sets the [padding](AView::mPadding).
     * @param padding padding
     * @details
     * @copydetails AView::mPadding
     */
    void setPadding(const ABoxFields& padding) {
        mPadding = padding;
    }

    /**
     * @brief String which helps to identify this object in debug string output (i.e., for logging)
     */
    virtual AString debugString() const;

    /**
     * @return pixel count which this AView's margin and padding acquired by width.
     */
    [[nodiscard]]
    int getTotalFieldHorizontal() const {
        return mPadding.horizontal() + mMargin.horizontal();
    }

    /**
     * @return pixel count which this AView's margin and padding acquired by height.
     */
    [[nodiscard]]
    int getTotalFieldVertical() const {
        return mPadding.vertical() + mMargin.vertical();
    }

    /**
     * @return pixel count which this AView's margin and padding acquired.
     */
    [[nodiscard]]
    glm::ivec2 getTotalFieldSize() const {
        return { getTotalFieldHorizontal(), getTotalFieldVertical() };
    }

    /**
     * @brief Parent AView.
     */
    AViewContainerBase* getParent() const
    {
        return mParent;
    }

    /**
     * @brief Determines shape which should pointer take when it's above this AView.
     */
    const AOptional<ACursor>& getCursor() const
    {
        return mCursor;
    }
    void setCursor(AOptional<ACursor> cursor);

    /**
     * @return minimal content-area width.
     */
    [[nodiscard]]
    virtual int getContentMinimumWidth();

    /**
     * @return minimal content-area height.
     */
    [[nodiscard]]
    virtual int getContentMinimumHeight();

    /**
     * @return minimal content-area size.
     */
    [[nodiscard]]
    glm::ivec2 getContentMinimumSize() noexcept {
        if (!mCachedMinContentSize) {
            glm::ivec2 minContentSize = glm::ivec2(getContentMinimumWidth(), getContentMinimumHeight());
            mCachedMinContentSize = minContentSize;
            return minContentSize;
        }
        return *mCachedMinContentSize;
    }

    [[nodiscard]]
    bool isContentMinimumSizeInvalidated() noexcept {
        return !mCachedMinContentSize.hasValue();
    }

    bool hasFocus() const;

    virtual int getMinimumWidth();
    virtual int getMinimumHeight();

    /**
     * @brief Returns the minimum size required for this view.
     * @return Minimum size (width, height) this view requires in pixels, excluding margins.
     * @details
     * The minimum size includes:
     * - Minimum content size
     * - Padding
     *
     * This value represents the absolute minimum dimensions the view needs to properly display its content. It's used
     * by layout managers to ensure views aren't sized smaller than what they require to be functional.
     */
    glm::ivec2 getMinimumSize() {
        return { getMinimumWidth(), getMinimumHeight() };
    }

    void setMaxSize(const glm::ivec2& maxSize) {
        mMaxSize = maxSize;
    }

    /**
     * @return maxSize (ignoring fixedSize)
     */
    [[nodiscard]] const glm::ivec2& getMaxSize() const
    {
        return mMaxSize;
    }

    /**
     * @return content size
     */
    glm::ivec2 getContentSize() const { return { getContentWidth(), getContentHeight() }; }

    int getContentWidth() const
    {
        return static_cast<int>(mSize.x - mPadding.horizontal());
    }

    int getContentHeight() const
    {
        return static_cast<int>(mSize.y - mPadding.vertical());
    }

    [[nodiscard]]
    const glm::ivec2& getExpanding() const
    {
        return mExpanding;
    }

    /**
     * @brief Changes the expanding of view.
     * @sa layout-managers
     * @sa mExpanding
     * @sa ass::Expanding
     */
    void setExpanding(glm::ivec2 expanding)
    {
        if (mExpanding == expanding) [[unlikely]] {
            return;
        }
        mExpanding = expanding;
        emit mExpandingChanged(expanding);
        markMinContentSizeInvalid();
    }

    /**
     * @brief Changes the expanding of view.
     * @sa layout-managers
     * @sa mExpanding
     * @sa ass::Expanding
     */
    void setExpanding(int expanding)
    {
        setExpanding(glm::ivec2(expanding));
    }
    void setExpanding()
    {
        setExpanding(2);
    }

    const _<AAnimator>& getAnimator() const {
        return mAnimator;
    }


    void setAnimator(const _<AAnimator>& animator);
    void getTransform(glm::mat4& transform) const;

    [[nodiscard]]
    int getExpandingHorizontal() const
    {
        return mExpanding.x;
    }

    [[nodiscard]]
    int getExpandingVertical() const
    {
        return mExpanding.y;
    }

    [[nodiscard]] aui::float_within_0_1 getOpacity() const {
        return mOpacity;
    }
    void setOpacity(aui::float_within_0_1 opacity) {
        mOpacity = opacity;
    }

    virtual void setPosition(glm::ivec2 position);

    /**
     * Set size ignoring all restrictions (i.e. min size, max size, fixed size, etc...). Used by AAnimator.
     * @param size
     */
    void setSizeForced(glm::ivec2 size) {
        mSize = size;
    }
    virtual void setSize(glm::ivec2 size);

    /**
     * @brief Sets position and size of the view.
     * @details
     * See [layout-managers] for more info.
     */
    virtual void setGeometry(int x, int y, int width, int height);
    void setGeometry(const glm::ivec2& position, const glm::ivec2& size) {
        setGeometry(position.x, position.y, size.x, size.y);
    }

    bool isBlockClicksWhenPressed() const noexcept {
        return mBlockClicksWhenPressed;
    }

    void setBlockClicksWhenPressed(bool value) noexcept {
        mBlockClicksWhenPressed = value;
    }

    /**
     * @brief Fixed size.
     * @return Fixed size. {0, 0} if unspecified.
     */
    const glm::ivec2& getFixedSize() {
        return mFixedSize;
    }
    void setFixedSize(glm::ivec2 size) {
        AUI_ASSERTX(glm::all(glm::greaterThanEqual(size, glm::ivec2(-100000))), "abnormal fixed size");
        if (size == mFixedSize) [[unlikely]] {
            return;
        }
        mFixedSize = size;
        markMinContentSizeInvalid();
    }

    [[nodiscard]]
    bool isMouseHover() const noexcept
    {
        return mHovered;
    }

    [[nodiscard]]
    bool isPressed() const noexcept
    {
        return !mPressed.empty();
    }

    [[nodiscard]]
    bool isPressed(APointerIndex index) const noexcept
    {
        return mPressed.contains(index);
    }

    bool isFocused() const {
        return mHasFocus;
    }
    bool isMouseEntered() const {
        return mMouseEntered;
    }

    Visibility getVisibility() const
    {
        return mVisibility;
    }

    Visibility getVisibilityRecursive() const;

    void setVisibility(Visibility visibility) noexcept;

    void setVisible(bool visible) noexcept
    {
        setVisibility(visible ? Visibility::VISIBLE : Visibility::INVISIBLE);
    }


    [[nodiscard]]
    const AColor& textColor() const {
        return mTextColor;
    }

    void setTextColor(AColor color) {
        mTextColor = color;
    }

    [[nodiscard]]
    MouseCollisionPolicy getMouseCollisionPolicy() const {
        return mMouseCollisionPolicy;
    }

    void setMouseCollisionPolicy(MouseCollisionPolicy mouseCollisionPolicy) {
        mMouseCollisionPolicy = mouseCollisionPolicy;
    }

    /**
     * Simulates click on the view. Useful then you want to call clicked() slots of this view.
     */
    void click() {
        emit clickedButton(APointerIndex::button(AInput::LBUTTON));
        emit clicked();
    }

    /**
     * @brief Sets minimal size.
     */
    void pack();

    /**
     * @brief Requests focus for this AView.
     * @param needFocusChainUpdate if true, focus chain for new focused view will be updated
     * @details
     * If needFocusChainUpdate is false you need to control focus chain targets outside the focus function
     */
     void focus(bool needFocusChainUpdate = true);

     /**
      * @return Can this view capture focus.
      * @details
      * For containers, capturing focus is redundant.
      */
     virtual bool capturesFocus();

    /**
     * @brief Checks if the specified view is an indirect parent of this view.
     */
    bool hasIndirectParent(const _<AView>& v);

    /**
     * @return Coords of this AView relative to window
     */
    [[nodiscard]] glm::ivec2 getPositionInWindow() const;

    /**
     * @brief Adds an ASS class to this AView.
     * @param assName new ASS name
     */
    void addAssName(const AString& assName);

    /**
     * @brief Removes an ASS class to this AView.
     * @param assName ASS name to remove
     */
    void removeAssName(const AString& assName);

    /**
     * @brief Depending on value, either adds or removes ass name.
     * @param assName ASS name to add or remove
     * @param value boolean that determines actual operation
     */
    void setAssName(const AString& assName, bool value) {
        if (value) {
            addAssName(assName);
        } else {
            removeAssName(assName);
        }
    }

    /**
     * @brief Wraps the addAssName function to make it easier to add ASS class names.
     * \example
     * <code>
     * ...
     * _new<ALabel>("Components") << ".components_title"
     * ...
     * </code>
     * @param assName new ASS name
     * @return this
     */
    inline AView& operator<<(const AString& assName) {
        addAssName(assName);
        return *this;
    }

    const _<AAssHelper>& getAssHelper() const {
        return mAssHelper;
    }

    const ass::PropertyListRecursive& getCustomAss() const {
        return mCustomStyleRule;
    }

    void setCustomStyle(ass::PropertyListRecursive rule);

    void ensureAssUpdated();

    /**
     * Handles touch screen gesture event.
     * @param origin position where the event(s) started to occur from.
     * @param event gesture event.
     * @see transformGestureEventsToDesktop
     * @return true, if consumed (handled). True value prevents click.
     * @details The standard implementation <code>AView::onGesture</code> emulates desktop events such as right click
     * and scroll.
     */
    virtual bool onGesture(const glm::ivec2& origin, const AGestureEvent& event);

    virtual void onMouseEnter();

    /**
     * @brief Handles pointer hover events
     * @param pos event position
     * @param event event description
     * @details
     * If the view is pressed, it would still received move events. Use AView::isMouseHover to check is the pointer
     * actually over view or not. See AView::onPointerReleased for more info.
     */
    virtual void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event);
    virtual void onMouseLeave();
    virtual void onDpiChanged();

    /**
     * @brief Called on pointer (mouse) released event.
     * @param event event
     */
    virtual void onPointerPressed(const APointerPressedEvent& event);

    /**
     * @brief Called on pointer (mouse) released event.
     * @param event event
     * @details
     * To handle clicks, you should use AView::clicked signal instead. View still receives pointer move and released
     * events even if cursor goes outside the view boundaries, or other exclusive event appeared (i.e. scrollarea
     * scroll). AView::clicked emitted only if release event occurred inside view and no other event has prevented
     * click gesture. See APointerReleasedEvent::triggerClick.
     */
    virtual void onPointerReleased(const APointerReleasedEvent& event);
    virtual void onPointerDoubleClicked(const APointerPressedEvent& event);

    /**
     * Handles mouse wheel events.
     * @param event event info.
     */
    virtual void onScroll(const AScrollEvent& event);
    virtual void onKeyDown(AInput::Key key);
    virtual void onKeyRepeat(AInput::Key key);
    virtual void onKeyUp(AInput::Key key);
    virtual void onFocusAcquired();
    virtual void onFocusLost();
    virtual void onCharEntered(AChar c);

    /**
     * @return true if this AView accepts tab focus
     */
    virtual bool handlesNonMouseNavigation();

    virtual void forceUpdateLayoutRecursively();

    virtual void setEnabled(bool enabled = true);

    void setDisabled(bool disabled = true) {
        setEnabled(!disabled);
    }

    void updateEnableState();

    void enable()
    {
        setEnabled(true);
    }
    void disable()
    {
        setEnabled(false);
    }

    /**
     * @brief Helper function for kAUI.h:AUI_WITH_STYLE
     */
    void operator&(ass::PropertyListRecursive rule) {
        setCustomStyle(std::move(rule));
    }

    /**
     * @brief Called on AWindowBase::preventClickOnPointerRelease.
     */
    virtual void onClickPrevented();

    /**
     * @brief Invalidates all styles, causing to iterate over all rules in global and parent stylesheets.
     * @details
     * Unlike invalidateStateStyles(), completely resets styles for this view, causing it to iterate over all rules in
     * global and parent stylesheets. This operation is much more expensive than invalidateStateStyles because
     * invalidateStateStyles iterates over a small set of rules and performs fewer checks.
     *
     * Prefer invalidateAllStyles over invalidateStateStyles when:
     * <ul>
     *   <li>Added/removed rules to applicable stylesheets</li>
     *   <li>The view is reinflated to other layout</li>
     *   <li>Added/removed/changed ass names of this or parent views</li>
     * </ul>
     */
    virtual void invalidateAllStyles();

    /**
     * @brief Updates state selectors for ASS.
     * @details
     * Unlike invalidateAllStyles, iterates on an already calculated small set of rules which is much more cheap that
     * invalidateAllStyles.
     *
     * Prefer invalidateStateStyles over invalidateAllStyles when:
     * <ul>
     *   <li>Changed state (hover, active, focus) of this view</li>
     * </ul>
     */
    void invalidateStateStyles() {
        invalidateStateStylesImpl(getMinimumSizePlusMargin());
    }

    /**
     * @brief Resets mAssHelper.
     */
    virtual void invalidateAssHelper();

    /**
     * @brief Returns true if view is textfield-like view which requires touchscreen keyboard when clicked.
     */
    [[nodiscard]]
    virtual bool wantsTouchscreenKeyboard();

    /**
     * @see AView::mSkipUntilLayoutUpdate
     */
    void setSkipUntilLayoutUpdate(bool skipUntilLayoutUpdate) {
        mSkipUntilLayoutUpdate = skipUntilLayoutUpdate;
    }

    /**
     * @brief Set floating value for AText.
     */
    void setFloating(AFloat f) noexcept
    {
        mFloating = f;
    }

    /**
     * @brief Floating value for AText.
     */
    [[nodiscard]]
    AFloat getFloating() const noexcept
    {
        return mFloating;
    }

signals:
    /**
     * @see onViewGraphSubtreeChanged()
     */
    emits<> viewGraphSubtreeChanged;

    /**
     * @brief View is painted onto the some surface.
     * @details
     * This signal is emitted when view's AView::postRender() is called. This signal can be used to keep track if view
     * is visible; however, AUI performs some optimizations when painting views. For example, a view located somewhere
     * in AScrollArea is not painted until it is outside of AScrollArea's frame or at least barely reaches it.
     */
    emits<> redrawn;

    emits<bool> hoveredState;
    emits<> mouseEnter;
    emits<> mouseLeave;

    emits<bool, APointerIndex> pressedState;
    emits<APointerIndex> pressed;
    emits<APointerIndex> released;

    /**
     * @brief Some mouse button clicked.
     */
    emits<APointerIndex> clickedButton;

    /**
     * @brief Left mouse button clicked.
     */
    emits<> clicked;

    /**
     * @brief Geometry (position and size) changed.
     */
    emits<glm::ivec2, glm::ivec2> geometryChanged;

    /**
     * @brief Scroll event.
     */
    emits<glm::ivec2> scrolled;

    /**
     * @brief Keyboard key pressed.
     */
    emits<AInput::Key> keyPressed;

    /**
     * @brief Keyboard key released.
     */
    emits<AInput::Key> keyReleased;

    /**
     * @brief Right mouse button clicked.
     */
    emits<> clickedRight;

    /**
     * @brief Right mouse button clicked or long press gesture applied.
     */
    emits<> clickedRightOrLongPressed;

    emits<APointerIndex> doubleClicked;

    emits<> customCssPropertyChanged;

    /**
     * @brief Focus state changed.
     * @param first whether focused or not.
     */
    emits<bool> focusState;
    emits<> focusAcquired;
    emits<> focusLost;

    emits<_<AView>> childFocused;

protected:
    /**
     * @brief Parent AView.
     */
    AViewContainerBase* mParent = nullptr;

    /**
     * @brief Drawing list, or baking drawing commands so that you don't have to parse the ASS every time.
     */
    std::array<ass::prop::IPropertyBase*, int(ass::prop::PropertySlot::COUNT)> mAss;

    /**
     * @brief Custom ASS Rules
     */
    ass::PropertyListRecursive mCustomStyleRule;

    /**
     * @brief Determines shape which should pointer take when it's above this AView.
     */
    AOptional<ACursor> mCursor = ACursor::DEFAULT;

    /**
     * @brief Top left corner's position relative to top left corner's position of the parent AView.
     */
    glm::ivec2 mPosition = { 0, 0 };

    /**
     * @brief Position changed.
     */
    emits<glm::ivec2> mPositionChanged;

    /**
     * @brief Size, including content area, border and padding.
     */
    glm::ivec2 mSize = { 20, 20 };

    /**
     * @brief Size changed.
     */
    emits<glm::ivec2> mSizeChanged;

    AOptional<glm::ivec2> mCachedMinContentSize;
    bool mMarkedMinContentSizeInvalid = false;

    /**
     * @brief Redraw requested flag for this particular view/
     * @details
     * This flag is set in redraw() method and reset in AView::render(ARenderContext context). redraw() method does not actually requests
     * redraw of window if mRedrawRequested. This approach ignores sequential redraw() calls if the view is not even
     * drawn.
     */
    bool mRedrawRequested = false;

    /**
     * @brief Minimal size.
     */
    glm::ivec2 mMinSize = {0, 0};

    /**
     * @brief Maximal size.
     */
    glm::ivec2 mMaxSize = {0x7fffff, 0x7fffff};

    /**
     * @brief Fixed size.
     */
    glm::ivec2 mFixedSize = {0, 0};

    /**
     * @brief Margin, which defines the spacing around this AView. Processed by the layout manager.
     * @details
     * See [aui-box-model].
     */
    ABoxFields mMargin;

    /**
     * @brief Padding, which defines the spacing around content area inside the view. Processed by AView implementation.
     * @details
     * See [aui-box-model].
     */
    ABoxFields mPadding;

    /**
     * @brief ASS class names.
     * @details
     * Needs keeping order.
     */
    AVector<AString> mAssNames;

    /**
     * @brief If set to true, AViewContainer is obligated ignore this view. This value is set to false by
     * AView::setGeometry.
     * @details
     * This flag addresses the issue when some container is filled with views by addView during several frames, causing
     * to draw them in wrong place (then their layout is not processed yet).
     */
    bool mSkipUntilLayoutUpdate = true;

    /**
     * @brief Converts touch screen events to desktop.
     * @param origin position where the event(s) started to occur from.
     * @param event gesture event.
     * @return true if consumed (handled).
     * @details
     * <dl>
     *   <dt><b>AFingerDragEvent</b></dt>
     *   <dd>Emulates mouse wheel scroll</dd>
     *   <dt><b>ALongPressEvent</b></dt>
     *   <dd>Shows context menu (if exists) or AView::clickedRightOrLongPressed</dd>
     * </dl>
     */
    bool transformGestureEventsToDesktop(const glm::ivec2& origin, const AGestureEvent& event);

    void applyAssRule(const ass::PropertyList& propertyList);
    void applyAssRule(const ass::PropertyListRecursive& propertyList);

    /**
     * @brief Produce context (right click) menu.
     * @return menu model
     */
    virtual AMenuModel composeContextMenu();

    /**
     * @brief Called when direct or indirect parent has changed.
     * @details
     * Called when a new direct or indirect parent was assigned or removed.
     *
     * If a subtree reattached atomically (i.e., there were no dangling state), the method is called once.
     *
     * The method is mostly intended to invalidate styles in order to respond to stylesheet rules (mExtraStylesheet) of
     * the new (in)direct parent.
     *
     * Emits viewGraphSubtreeChanged signal.
     */
    virtual void onViewGraphSubtreeChanged();

    /**
     * @brief A view requests to redraw it and passes it's coords relative to this.
     * @param invalidArea area to invalidate. Must be in this view's coordinate space.
     */
    virtual void markPixelDataInvalid(ARect<int> invalidArea);

    virtual void commitStyle();

private:
    /**
     * @brief Animation.
     */
    _<AAnimator> mAnimator;

    /**
     * @brief Controls how does the overflow (stencil) mask is produced.
     */
    AOverflowMask mOverflowMask = AOverflowMask::ROUNDED_RECT;

    /**
     * @see Visibility
     */
    Visibility mVisibility = Visibility::VISIBLE;
    emits<Visibility> mVisibilityChanged;

    AColor mTextColor;

    /**
     * @brief Helper middleware object for handling ASS state updates (hover, active, etc...)
     */
    _<AAssHelper> mAssHelper;

    /**
     * @brief border-radius, specified in ASS.
     */
    float mBorderRadius = 0;

    /**
     * @brief Mouse collision policy. See MouseCollisionPolicy.
     */
    MouseCollisionPolicy mMouseCollisionPolicy = MouseCollisionPolicy::DEFAULT;

    /**
     * @brief opacity, specified in ASS.
     */
    aui::float_within_0_1 mOpacity = 1;

    /**
     * @brief Determines whether display graphics that go out of the bounds of this AView or not.
     */
    AOverflow mOverflow = AOverflow::VISIBLE;

    /**
     * @brief Extra stylesheet, specified by declarative::Style.
     * @details
     * Extra stylesheet is applied for this view and all his children recursively.
     *
     * Unlike custom style, extra stylesheet has multiple rules with selectors and it applies for the children
     * recursively.
     *
     * Extra stylesheet overrides the global stylesheet on conflicts.
     */
    _<AStylesheet> mExtraStylesheet;

    /**
     * @brief Called when parent's enable state is changed. Overridden in AViewContainer.
     * @param enabled
     */
    virtual void notifyParentEnabledStateChanged(bool enabled);

    /**
     * @brief Returns parent layout direction. If there's no parent, or parent does not have layout,
     *        ALayoutDirection::NONE returned.
     */
    [[nodiscard]]
    ALayoutDirection parentLayoutDirection() const noexcept;

    /**
     * @brief True if last called function among onMouseEnter and onMouseLeave is onMouseEnter, false otherwise
     * @details
     * This flag is used to avoid extra calls of onMouseEnter and onMouseLeave when hover is disabledn
     */
    bool mMouseEntered = false;

    /**
     * @brief Determines if pressing the view allows triggering click on other views
     * @details
     * By default on mobile platforms AUI will block clicks if there more than one pointer on screen,
     * if this flag is set to false, allows to click on others views without releasing pointer from this view
     */
    bool mBlockClicksWhenPressed = true;

    AFieldSignalEmitter<bool> mHovered = AFieldSignalEmitter<bool>(hoveredState, mouseEnter, mouseLeave);
    ASmallVector<APointerIndex, 1> mPressed;

    bool mEnabled = true;
    emits<bool> mEnabledChanged;

    bool mDirectlyEnabled = true;
    bool mParentEnabled = true;
    AFieldSignalEmitter<bool> mHasFocus = AFieldSignalEmitter<bool>(focusState, focusAcquired, focusLost, false);

    glm::ivec2 mExpanding = glm::ivec2{0, 0};
    emits<glm::ivec2> mExpandingChanged;

    /**
     * @brief Floating value for AText.
     */
    AFloat mFloating = AFloat::NONE;

    struct RenderToTexture {
        _unique<IRenderViewToTexture> rendererInterface;
        IRenderViewToTexture::InvalidArea invalidArea;

        bool drawFromTexture = true;

        /**
         * @brief Helps avoiding unwanted redrawing if RenderToTexture-capable view is not actually visible.
         */
        bool skipRedrawUntilTextureIsPresented = false;
    };
    AOptional<RenderToTexture> mRenderToTexture;

    /**
     * @brief Applies state-dependent styles and invalidates pixel data, layout, repaint if needed.
     */
    virtual void invalidateStateStylesImpl(glm::ivec2 prevMinimumSizePlusField);

    void notifyParentChildFocused(const _<AView>& view);
};

API_AUI_VIEWS std::ostream& operator<<(std::ostream& os, const AView& view);

template <typename Factory>
requires aui::not_overloaded_lambda<Factory> && aui::factory<Factory, _<AView>>
struct aui::implicit_shared_ptr_ctor<Factory> {
    auto operator()(Factory&& factory) {
        auto view = std::invoke(std::forward<Factory>(factory));
        view->addAssName(AClass<std::decay_t<Factory>>::name());
        return view;
    }
};
