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
    bool mIsPasswordTextField = false;

	void invalidatePrerenderedString() override;

	AString getContentsPasswordWrap();

protected:
    Render::PrerenderedString mPrerenderedString;
    AString mContents;
	virtual bool isValidText(const AString& text);

    void prerenderStringIfNeeded();

    void typeableErase(size_t begin, size_t end) override;
    void typeableInsert(size_t at, const AString& toInsert) override;
    size_t typeableFind(wchar_t c, size_t startPos) override;
    size_t typeableReverseFind(wchar_t c, size_t startPos) override;
    size_t length() const override;

    void typeableInsert(size_t at, wchar_t toInsert) override;

    AString getDisplayText() override;

    void doRedraw() override;


public:
	AAbstractTextField();
	virtual ~AAbstractTextField();

	int getContentMinimumHeight() override;
	void setText(const AString& t) override;

    void render() override;

    void setPasswordMode(bool isPasswordMode) {
        mIsPasswordTextField = isPasswordMode;
    }

    bool handlesNonMouseNavigation() override;
    void onFocusAcquired() override;

    AString getText() const override;

    void onCharEntered(wchar_t c) override;
    void invalidateFont() override;

};
