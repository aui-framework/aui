/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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

#pragma once

#include <AUI/Util/ACursorSelectable.h>
#include "AView.h"
#include "AUI/Common/ATimer.h"
#include <AUI/Render/Render.h>

class API_AUI_VIEWS AAbstractTextField : public AView, public ACursorSelectable
{
private:
	AString mContents;
	Render::PrerenderedString mPrerenderedString;

	static ATimer& blinkTimer();
	
	unsigned mCursorBlinkCount = 0;
	bool mCursorBlinkVisible = true;
	bool mTextChangedFlag = false;
    bool mIsPasswordTextField = false;
    bool mIsMultiline = false;

	int mHorizontalScroll = 0;
	size_t mMaxTextLength = 0x200;

	void updateCursorBlinking();
	void updateCursorPos();
	void invalidatePrerenderedString() {
        mPrerenderedString.mVao = nullptr;
	}

    AString getContentsPasswordWrap();
	
protected:
	virtual bool isValidText(const AString& text) = 0;

    glm::ivec2 getMouseSelectionPadding() override;
    glm::ivec2 getMouseSelectionScroll() override;
    FontStyle getMouseSelectionFont() override;
    AString getMouseSelectionText() override;

    void doRedraw() override;

public:
	AAbstractTextField();
	virtual ~AAbstractTextField();

	int getContentMinimumHeight() override;

	void onKeyDown(AInput::Key key) override;
	void onKeyRepeat(AInput::Key key) override;

	void onCharEntered(wchar_t c) override;
	void render() override;

    void invalidateFont() override;

    void onFocusLost() override;
	void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button) override;

    void onMouseMove(glm::ivec2 pos) override;
	void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

	void setText(const AString& t);

	[[nodiscard]] const AString& getText() const override
	{
		return mContents;
	}

    void setPasswordMode(bool isPasswordMode) {
        mIsPasswordTextField = isPasswordMode;
    }

	void setMaxTextLength(size_t newTextLength) {
	    mMaxTextLength = newTextLength;
	}

    bool handlesNonMouseNavigation() override;
    void onFocusAcquired() override;

signals:
	/**
	 * \brief Text changed.
	 * \note This signal is also emitted by the AAbstractTextField::setText function
	 */
	emits<AString> textChanged;

	/**
	 * \brief Text is changing by the user.
	 */
	emits<AString> textChanging;
};
