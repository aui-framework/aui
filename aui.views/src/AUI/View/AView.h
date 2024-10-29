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


class AWindow;
class ABaseWindow;
class AViewContainer;
class AAnimator;
class AAssHelper;
class AStylesheet;

/**
 * @defgroup useful_views Views
 * @ingroup views
 * @brief All views that ready to use
 */

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
 * @ingroup useful_views
 */
class API_AUI_VIEWS AView: public AObject
{
    friend class AViewContainer;
    friend class IRenderViewToTexture;
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
     * @note This flag is used to avoid extra calls of onMouseEnter and onMouseLeave when hover is disabled
     */
    bool mMouseEntered = false;

    /**
     * @brief Determines if pressing the view allows triggering click on other views
     * @note By default on mobile platforms AUI will block clicks if there more than one pointer on screen,
     *       if this flag is set to false, allows to click on others views without releasing pointer from this view
     */
    bool mBlockClicksWhenPressed = true;

protected:
    /**
     * @brief Parent AView.
     */
    AViewContainer* mParent = nullptr;

    /**
     * @brief Determines how to display text that go out of the bounds.
     */
    ATextOverflow mTextOverflow = ATextOverflow::NONE;

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
     * @brief Size, including content area, border and padding.
     */
    glm::ivec2 mSize = { 20, 20 };

    /**
     * @brief Expansion coefficient. Hints layout manager how much this AView should be extended relative to other
     *        AViews in the same container.
     * @details
     *
     * It does affect expanding environment inside the container. See expanding @ref layout_managers "layout managers"
     * for more info.
     *
     * It does not affect parent's size or parent's expanding property. Use AView::setExpanding() on parent, or ::Expanding
     * variant of declarative container notation (Vertical::Expanding, Horizontal::Expanding, Stacked::Expanding) for
     * such case.
     */
    glm::ivec2 mExpanding = {0, 0};

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
    glm::ivec2 mMaxSize = {0x7fffffff, 0x7fffffff};

    /**
     * @brief Fixed size.
     */
    glm::ivec2 mFixedSize = {0, 0};

    /**
     * @brief Margin, which defines the spacing around this AView. Processed by the layout manager.
     */
    ABoxFields mMargin;

    /**
     * @brief Padding, which defines the spacing around content area inside the view. Processed by AView implementation.
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
     * The method is mostly intended to invalidate styles in order to respond to stylesheet rules (mExtraStylesheet) of
     * the new (in)direct parent.
     *
     * Emits viewGraphSubtreeChanged signal.
     */
    virtual void onViewGraphSubtreeChanged();


    /**
     * @brief A view requests to redraw it and passes it's coords relative to this.
     * @param invalidArea area to invalidate. Must be in this view's coordinate space.
     * @return A window that manages this invalidation event.
     */
    virtual void markPixelDataInvalid(ARect<int> invalidArea);

    virtual void commitStyle();

public:
    AView();
    ~AView() override;
    /**
     * @brief Request window manager to redraw this AView.
     */
    void redraw();

    /**
     * @brief Determines window which this AView belongs to.
     * @return window which this AView belongs to. Could be nullptr
     */
    ABaseWindow* getWindow() const;

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
     * @code{cpp}
     * mWindow->onPointerMove(mView->getCenterPointInWindow()); // triggers on pointer move over the view through window
     * @endcode
     */
    [[nodiscard]]
    glm::ivec2 getCenterPointInWindow() const noexcept
    {
        return getPositionInWindow() + getSize() / 2;
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

    void setTextOverflow(ATextOverflow textOverflow) {
        mTextOverflow = textOverflow;
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
    float getTotalOccupiedWidth() const
    {
        return mVisibility == Visibility::GONE ? 0 : mSize.x + getTotalFieldHorizontal();
    }

    /**
     * @return pixel count which this AView acquired by height including content area, padding, border and margin.
     */
    float getTotalOccupiedHeight() const
    {
        return mVisibility == Visibility::GONE ? 0 : mSize.y + getTotalFieldVertical();
    }

    [[nodiscard]]
    glm::ivec2 getMinimumSizePlusField() {
        return getMinimumSize() + getTotalFieldSize();
    }

    /**
     * @brief Returns the @ref AView::mMargin "margin".
     * @return margin
     */
    [[nodiscard]]
    const ABoxFields& getMargin() const
    {
        return mMargin;
    }

    /**
     * @brief Sets the @ref AView::mMargin "margin".
     * @param margin margin
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
     * @brief Returns the @ref AView::mPadding "padding".
     * @return padding
     */
    [[nodiscard]]
    const ABoxFields& getPadding() const
    {
        return mPadding;
    }

    /**
     * @brief Sets the @ref AView::mPadding "padding".
     * @param padding padding
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
    AViewContainer* getParent() const
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
    virtual int getContentMinimumWidth(ALayoutDirection layout);


    /**
     * @return minimal content-area height.
     */
    [[nodiscard]]
    virtual int getContentMinimumHeight(ALayoutDirection layout);

    /**
     * @return minimal content-area size.
     */
    [[nodiscard]]
    glm::ivec2 getContentMinimumSize(ALayoutDirection layout) noexcept {
        if (!mCachedMinContentSize) {
            glm::ivec2 minContentSize = glm::ivec2(getContentMinimumWidth(layout), getContentMinimumHeight(layout));
            mCachedMinContentSize = minContentSize;
            return minContentSize;
        }
        // TODO ignore layout?
        return *mCachedMinContentSize;
    }

    [[nodiscard]]
    bool isContentMinimumSizeInvalidated() noexcept {
        return !mCachedMinContentSize.hasValue();
    }


    bool hasFocus() const;


    virtual int getMinimumWidth(ALayoutDirection layout = ALayoutDirection::NONE);
    virtual int getMinimumHeight(ALayoutDirection layout = ALayoutDirection::NONE);

    glm::ivec2 getMinimumSize() {
        return {getMinimumWidth(), getMinimumHeight()};
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
     * @sa mExpanding
     * @sa ass::Expanding
     */
    void setExpanding(glm::ivec2 expanding)
    {
        if (mExpanding == expanding) [[unlikely]] {
            return;
        }
        mExpanding = expanding;
        markMinContentSizeInvalid();
    }
    void setExpanding(int expanding)
    {
        setExpanding({expanding, expanding});
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

    [[nodiscard]]
    bool isEnabled() const noexcept
    {
        return mEnabled;
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
     * @note if needFocusChainUpdate is false you need to control focus chain targets outside the focus function
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


    [[nodiscard]]
    _<AView> sharedPtr() {
        return _cast<AView>(AObject::sharedPtr());
    }

    [[nodiscard]]
    _weak<AView> weakPtr() {
        return _weak<AView>(sharedPtr());
    }

    /**
     * Handles touch screen gesture event.
     * @param origin position where the event(s) started to occur from.
     * @param event gesture event.
     * @note The standard implementation <code>AView::onGesture</code> emulates desktop events such as right click and
     *       scroll.
     * @see transformGestureEventsToDesktop
     * @return true, if consumed (handled). True value prevents click.
     */
    virtual bool onGesture(const glm::ivec2& origin, const AGestureEvent& event);

    virtual void onMouseEnter();

    /**
     * @brief Handles pointer hover events
     * @param pos event position
     * @details
     * @note
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
     * @note
     * To handle clicks, you should use AView::clicked signal instead. View still receives pointer move and released
     * events even if cursor goes outside the view boundaries, or other exclusive event appeared (i.e. scrollarea
     * scroll). AView::clicked emitted only if release event occurred inside view and no other event has prevented
     * click gesture. See APointerReleasedEvent::triggerClick.
     */
    virtual void onPointerReleased(const APointerReleasedEvent& event);
    virtual void onPointerDoubleClicked(const APointerPressedEvent& event);

    /**
     * Handles mouse wheel events.
     * @param pos mouse cursor position.
     * @param delta the distance mouse wheel scrolled.
     * @details
     * By default, 120 is single mouse wheel click.
     * 120 = mouse scroll down, -120 = mouse scroll up.
     */
    virtual void onScroll(const AScrollEvent& event);
    virtual void onKeyDown(AInput::Key key);
    virtual void onKeyRepeat(AInput::Key key);
    virtual void onKeyUp(AInput::Key key);
    virtual void onFocusAcquired();
    virtual void onFocusLost();
    virtual void onCharEntered(char16_t c);
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
     * @brief Helper function for kAUI.h:with_style
     */
    void operator+(ass::PropertyListRecursive rule) {
        setCustomStyle(std::move(rule));
    }

    /**
     * @brief Called on ABaseWindow::preventClickOnPointerRelease.
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
        invalidateStateStylesImpl(getMinimumSizePlusField());
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

signals:
    /**
     * @see onViewGraphSubtreeChanged()
     */
    emits<> viewGraphSubtreeChanged;

    emits<bool> hoveredState;
    emits<> mouseEnter;
    emits<> mouseLeave;

    emits<bool, APointerIndex> pressedState;
    emits<APointerIndex> pressed;
    emits<APointerIndex> released;

    emits<bool> enabledState;
    emits<> enabled;
    emits<> disabled;

    /**
     * @brief Some mouse button clicked.
     */
    emits<APointerIndex> clickedButton;

    /**
     * @brief Left mouse button clicked.
     */
    emits<> clicked;

    /**
     * @brief Position changed.
     */
    emits<glm::ivec2> positionChanged;

    /**
     * @brief Size changed.
     */
    emits<glm::ivec2> sizeChanged;

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

private:
    AFieldSignalEmitter<bool> mHovered = AFieldSignalEmitter<bool>(hoveredState, mouseEnter, mouseLeave);
    ASmallVector<APointerIndex, 1> mPressed;
    //AWatchable<bool> mFocused = AWatchable<bool>(pressedState, pressed, released);
    AFieldSignalEmitter<bool> mEnabled = AFieldSignalEmitter<bool>(enabledState, enabled, disabled, true);
    bool mDirectlyEnabled = true;
    bool mParentEnabled = true;
    AFieldSignalEmitter<bool> mHasFocus = AFieldSignalEmitter<bool>(focusState, focusAcquired, focusLost, false);

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