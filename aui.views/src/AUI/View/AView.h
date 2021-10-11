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

#pragma once
#include <glm/glm.hpp>

#include <AUI/ASS/Declaration/IDeclaration.h>
#include "AUI/Common/ABoxFields.h"
#include "AUI/Common/ADeque.h"
#include "AUI/Common/AObject.h"
#include "AUI/Common/AVariant.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/Platform/ACursor.h"
#include "AUI/Platform/AInput.h"
#include "AUI/Reflect/AClass.h"
#include "AUI/Render/FontStyle.h"
#include "AUI/Util/Watchable.h"
#include "AUI/Util/IShadingEffect.h"
#include <AUI/ASS/RuleWithoutSelector.h>
#include <AUI/Enum/Overflow.h>
#include <AUI/Enum/Visibility.h>

#include <chrono>
#include <functional>
#include <array>

class Render;
class AWindow;
class AViewContainer;
class AAnimator;
class AAssHelper;


/**
 * \brief A class that describes the minimum unit that can be placed in a container (including a window) that takes up
 *        some space on the screen that responds to changes in size, position, moving the cursor, pressing / releasing
 *        keys and buttons, movement mouse wheel, etc...
 *
 *        Analogue to QWidget, View.
 */
class API_AUI_VIEWS AView: public AObject
{
	friend class AViewContainer;
private:

	/**
	 * \brief Animation.
	 */
	_<AAnimator> mAnimator;

	/**
	 * \brief Determines whether display graphics that go out of the bounds of this AView or not.
	 */
	Overflow mOverflow = Overflow::VISIBLE;

	/**
	 * \see Visibility
	 */
	Visibility mVisibility = Visibility::VISIBLE;

    /**
     * \brief Helper middleware object for handling ASS state updates (hover, active, etc...)
     */
    _<AAssHelper> mAssHelper;

	/**
	 * \brief Background effects (custom rendered backgrounds)
	 */
	ADeque<_<IShadingEffect>> mBackgroundEffects;

	/**
	 * \brief border-radius, specified in ASS.
	 */
    float mBorderRadius = 0;

	/**
	 * \brief Font style for this AView.
	 */
	FontStyle mFontStyle;

    /**
     * \brief opacity, specified in ASS.
     */
    float mOpacity = 1;

    virtual void notifyParentEnabledStateChanged(bool enabled);

protected:
	/**
	 * \brief Parent AView.
	 */
	AViewContainer* mParent = nullptr;


    /**
     * \brief Drawing list, or baking drawing commands so that you don't have to parse the ASS every time.
     */
    std::array<ass::decl::IDeclarationBase*, int(ass::decl::DeclarationSlot::COUNT)> mAss;

    /**
     * \brief Custom ASS Rules
     */
    RuleWithoutSelector mCustomAssRule;

	/**
	 * \brief Determines shape which should pointer take when it's above this AView.
	 */
	ACursor mCursor = ACursor::DEFAULT;

	/**
	 * \brief Top left corner's position relative to top left corner's position of the parent AView.
	 */
	glm::ivec2 mPosition;

	/**
	 * \brief Size, including content area, border and padding.
	 */
	glm::ivec2 mSize = glm::ivec2(20, 20);

	/**
	 * \brief Expansion coefficient. Hints layout manager how much this AView should be extended relative to other
	 *        AViews in the same container.
	 */
	glm::ivec2 mExpanding = {0, 0};

	/**
	 * \brief Minimal size.
	 */
	glm::ivec2 mMinSize = {0, 0};

	/**
	 * \brief Maximal size.
	 */
	glm::ivec2 mMaxSize = {0x7fffffff, 0x7fffffff};

	/**
	 * \brief Fixed size.
	 */
	glm::ivec2 mFixedSize = {0, 0};

	/**
	 * \brief Margin, which defines the spacing around this AView. Processed by the layout manager.
	 */
	ABoxFields mMargin;

	/**
	 * \brief Padding, which defines the spacing around content area. Processed by AView implementation.
	 */
	ABoxFields mPadding;


	/**
	 * \brief ASS class names.
	 */
	ADeque<AString> mAssNames;

	/**
	 * \brief Determines window which this AView belongs to.
	 * \return window which this AView belongs to. Could be nullptr
	 */
	AWindow* getWindow();

	/**
	 * \brief Determines which ASS style rules should be applied to this AView and fills the mAss field.
	 */
	virtual void recompileCSS();

	/**
	 * \brief Updates state selectors for ASS.
	 */
	void updateAssState();


public:
    AView();
    virtual ~AView() = default;
	/**
	 * \brief Request window manager to redraw this AView.
	 */
	void redraw();

	virtual void drawStencilMask();


	/**
	 * \brief Draws this AView. Noone should call this function except rendering routine.
	 */
	virtual void render();

	virtual void postRender();

	void popStencilIfNeeded();

	[[nodiscard]]
    const ADeque<AString>& getAssNames() const {
        return mAssNames;
    }

    /**
     * \brief Top left corner's position relative to top left corner's position of the parent AView.
     */
	const glm::ivec2& getPosition() const

	{
		return mPosition;
	}

    /**
     * \brief Size, including content area, border and padding.
     */
	const glm::ivec2& getSize() const
	{
		return mSize;
	}

    /**
     * \brief Minimal size.
     */
    const glm::ivec2& getMinSize() const {
        return mMinSize;
    }

    void setMinSize(const glm::ivec2& minSize) {
        mMinSize = minSize;
    }

    /**
     * \brief Determines whether display graphics that go out of the bounds of this AView or not.
     */
    Overflow getOverflow() const
	{
		return mOverflow;
	}
    void setOverflow(Overflow overflow)
	{
		mOverflow = overflow;
	}

    /**
     * \brief border-radius, specified in ASS.
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
	 * \return pixel count which this AView acquired by width including content area, padding, border and margin.
	 */
	float getTotalOccupiedWidth() const
	{
		return mVisibility == Visibility::GONE ? 0 : mSize.x + getTotalFieldHorizontal();
	}

    /**
	 * \return pixel count which this AView acquired by height including content area, padding, border and margin.
     */
    float getTotalOccupiedHeight() const
	{
        return mVisibility == Visibility::GONE ? 0 : mSize.y + getTotalFieldVertical();
	}

    /**
     * \brief Margin, which defines the spacing around this AView. Processed by the layout manager.
     */
	[[nodiscard]]
	const ABoxFields& getMargin() const
	{
		return mMargin;
	}
    void setMargin(const ABoxFields& margin) {
        mMargin = margin;
    }

    /**
     * \brief Determines whether this AView processes this click or passes it thru.
     * \param pos mouse position
     * \return true if AView processes this click
     */
	virtual bool consumesClick(const glm::ivec2& pos);

    /**
     * \brief Padding, which defines the spacing around content area. Processed by AView implementation.
     */
	[[nodiscard]]
	const ABoxFields& getPadding() const
	{
		return mPadding;
	}

	void setPadding(const ABoxFields& padding) {
	    mPadding = padding;
	}


	/**
	 * \return pixel count which this AView's margin and padding acquired by width.
	 */
	[[nodiscard]]
	float getTotalFieldHorizontal() const;

	/**
	 * \return pixel count which this AView's margin and padding acquired by height.
	 */
	[[nodiscard]]
	float getTotalFieldVertical() const;


    /**
     * \brief Parent AView.
     */
	AViewContainer* getParent() const
	{
		return mParent;
	}

    /**
     * \brief Determines shape which should pointer take when it's above this AView.
     */
	ACursor getCursor() const
	{
		return mCursor;
	}
	void setCursor(ACursor cursor)
	{
		mCursor = cursor;
	}

	/**
	 * \return minimal content-area width.
	 */
	virtual int getContentMinimumWidth();


    /**
     * \return minimal content-area height.
     */
	virtual int getContentMinimumHeight();

	bool hasFocus() const;


	virtual int getMinimumWidth();
    virtual int getMinimumHeight();

	glm::ivec2 getMinimumSize() {
	    return {getMinimumWidth(), getMinimumHeight()};
	}

    void setMaxSize(const glm::ivec2& maxSize) {
        mMaxSize = maxSize;
    }
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
	void setExpanding(const glm::ivec2& expanding)
	{
		mExpanding = expanding;
	}
    void setExpanding()
    {
        mExpanding = glm::ivec2(2);
    }

    const _<AAnimator>& getAnimator() const {
	    return mAnimator;
	}

    void setAnimator(const _<AAnimator>& animator);
    void getTransform(glm::mat4& transform) const;

    int getExpandingHorizontal() const
	{
		return mExpanding.x;
	}
    int getExpandingVertical() const
	{
		return mExpanding.y;
	}
	FontStyle& getFontStyle();

    [[nodiscard]] float getOpacity() const {
        return mOpacity;
    }
    void setOpacity(float opacity) {
        mOpacity = opacity;
    }

    virtual void invalidateFont();
	virtual void setPosition(const glm::ivec2& position);

	void setSize(const glm::ivec2& size) {
        setSize(size.x, size.y);
	}

    /**
     * Set size ignoring all restrictions (i.e. min size, max size, fixed size, etc...). Used by AAnimator.
     * @param size
     */
	void setSizeForced(const glm::ivec2& size) {
        mSize = size;
	}
    virtual void setSize(int width, int height);
    virtual void setGeometry(int x, int y, int width, int height);


    /**
     * \brief Fixed size.
     */
    const glm::ivec2& getFixedSize() {
	    return mFixedSize;
	}
	void setFixedSize(const glm::ivec2& size) {
	    mFixedSize = size;
	}

	bool isMouseHover() const
	{
		return mHovered;
	}

    bool isMousePressed() const
	{
		return mPressed;
	}
    bool isEnabled() const
	{
		return mEnabled;
	}
	bool isFocused() const {
        return mHasFocus;
    }
	Visibility getVisibility() const
	{
		return mVisibility;
	}
	Visibility getVisibilityRecursive() const;

	void setVisibility(Visibility visibility)
	{
		mVisibility = visibility;
		redraw();
	}

    /**
     * Simulates click on the view. Useful then you want to call clicked() slots of this view.
     */
    void click() {
        emit clickedButton(AInput::LButton);
        emit clicked();
    }

	/**
	 * \brief Sets minimal size.
	 */
	void pack();

	/**
	 * \brief Requests focus for this AView. It's overridden for AViewContainer.
	 */
	 virtual void focus();


	/**
	 * \return Coords of this AView relative to window
	 */
    [[nodiscard]] glm::ivec2 getPositionInWindow() const;

	const ADeque<AString>& getCssNames() const;

	/**
	 * \brief Adds an ASS class to this AView.
	 * \param assName new ASS name
	 */
    void addAssName(const AString& assName);

	/**
	 * \brief Removes an ASS class to this AView.
	 * \param assName ASS name to remove
	 */
    void removeAssName(const AString& assName);

	/**
	 * \brief Wraps the addAssName function to make it easier to add ASS class names.
	 * \example
	 * <code>
	 * ...
	 * _new<ALabel>("Components") << ".components_title"
	 * ...
	 * </code>
	 * \param assName new ASS name
	 * \return this
	 */
	inline AView& operator<<(const AString& assName) {
        addAssName(assName);
	    return *this;
	}

	const _<AAssHelper>& getAssHelper() const {
	    return mAssHelper;
	}

	const RuleWithoutSelector& getCustomAss() const {
		return mCustomAssRule;
	}

	void setCustomAss(const RuleWithoutSelector& rule) {
		mCustomAssRule = rule;
	}

    void ensureAssUpdated();

    /**
     * \brief Tries to determine std::shared_ptr for this object.
     */
    virtual _<AView> determineSharedPointer();

	virtual void onMouseEnter();
    virtual void onMouseMove(glm::ivec2 pos);
    virtual void onMouseLeave();
    virtual void onDpiChanged();

	virtual void onMousePressed(glm::ivec2 pos, AInput::Key button);
    virtual void onMouseReleased(glm::ivec2 pos, AInput::Key button);
    virtual void onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button);

    /**
     * Handles mouse wheel events
     * @param pos mouse cursor position
     * @param delta the distance mouse wheel scrolled. 120 = mouse scroll down, -120 = mouse scroll up.
     */
    virtual void onMouseWheel(glm::ivec2 pos, int delta);
    virtual void onKeyDown(AInput::Key key);
    virtual void onKeyRepeat(AInput::Key key);
    virtual void onKeyUp(AInput::Key key);
    virtual void onFocusAcquired();
    virtual void onFocusLost();

	virtual void onCharEntered(wchar_t c);

	virtual void getCustomCssAttributes(AMap<AString, AVariant>& map);

	/**
	 * \return true if this AView accepts tab focus
	 */
	virtual bool handlesNonMouseNavigation();

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
	 * Helper function for kAUI.h:with_style
	 */
    void operator+(const RuleWithoutSelector& rule) {
        setCustomAss(rule);
    }

signals:
    emits<bool> hoveredState;
    emits<> mouseEnter;
    emits<> mouseLeave;

	emits<bool> pressedState;
    emits<> mousePressed;
    emits<> mouseReleased;

	emits<bool> enabledState;
    emits<> enabled;
    emits<> disabled;

	/**
	 * \brief Some mouse button clicked.
	 */
	emits<AInput::Key> clickedButton;

	/**
	 * \brief Left mouse button clicked.
	 */
	emits<> clicked;

	/**
	 * \brief Right mouse button clicked.
	 */
	emits<> clickedRight;

	emits<AInput::Key> doubleClicked;

	emits<> customCssPropertyChanged;

	emits<bool> focusState;
	emits<> focusAcquired;
	emits<> focusLost;

private:
	Watchable<bool> mHovered = Watchable<bool>(hoveredState, mouseEnter, mouseLeave);
	Watchable<bool> mPressed = Watchable<bool>(pressedState, mousePressed, mouseReleased);
	//Watchable<bool> mFocused = Watchable<bool>(pressedState, mousePressed, mouseReleased);
	Watchable<bool> mEnabled = Watchable<bool>(enabledState, enabled, disabled, true);
    bool mDirectlyEnabled = true;
    bool mParentEnabled = true;
	Watchable<bool> mHasFocus = Watchable<bool>(focusState, focusAcquired, focusLost, false);
};
