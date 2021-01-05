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
#include "AView.h"
#include "AUI/Render/Render.h"
#include "AUI/Common/AString.h"
#include "AUI/Image/IDrawable.h"

class API_AUI_VIEWS ALabel: public AView
{
private:
	AString mText;
	_<IDrawable> mIcon;
    _<AFont> mFontOverride;
    uint8_t mFontSizeOverride = 0;
    TextAlign mVerticalAlign;
    enum {
        TT_NORMAL,
        TT_UPPERCASE,
        TT_LOWERCASE
    } mTextTransform = TT_NORMAL;
	bool mMultiline = false;

protected:
    AStringVector mLines;
    Render::PrerenderedString mPrerendered;

	void updateMultiline();
	FontStyle getFontStyleLabel();

	const Render::PrerenderedString& getPrerendered() {
	    return mPrerendered;
	}

    //void userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) override;
    void doRenderText();
    AString getTargetText();


    // for correct selection positioning (used in ASelectableLabel)
    int mTextLeftOffset = 0;

public:
	ALabel();
	explicit ALabel(const AString& text);

	void render() override;

	int getContentMinimumWidth() override;
	int getContentMinimumHeight() override;

    void invalidateFont() override;

    const _<IDrawable>& getIcon() const
	{
		return mIcon;
	}

	void setIcon(const _<IDrawable>& drawable) {
        mIcon = drawable;
        redraw();
    }
    void doPrerender();


public:
    void onDpiChanged() override;

    void setText(const AString& newText);


	[[nodiscard]] bool isMultiline() const
	{
		return mMultiline;
	}

	[[nodiscard]] const AString& getText() const
	{
		return mText;
	}

	void setMultiline(const bool multiline);
	void setFont(_<AFont> font) {
	    mFontOverride = font;
	}
	void setFontSize(uint8_t size) {
        mFontSizeOverride = size;
    }

    void setSize(int width, int height) override;

};