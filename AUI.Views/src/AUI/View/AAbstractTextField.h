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

#pragma once

#include <AUI/View/AAbstractTypeableView.h>
#include "AView.h"
#include "AUI/Common/ATimer.h"
#include <AUI/Render/Render.h>

class API_AUI_VIEWS AAbstractTextField : public AAbstractTypeableView
{
private:
	AString mContents;
	Render::PrerenderedString mPrerenderedString;

	void invalidatePrerenderedString() {
        mPrerenderedString.mVao = nullptr;
	}
	
protected:
	virtual bool isValidText(const AString& text) = 0;



public:
	AAbstractTextField();
	virtual ~AAbstractTextField();

	int getContentMinimumHeight() override;


	void setText(const AString& t);
	void clear() {
	    setText({});
	}
	void trimText() {
	    setText(getText().trim());
	}

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

    void selectAll();

    void copyToClipboard() const;

    void cutToClipboard();

    void pasteFromClipboard();
};
