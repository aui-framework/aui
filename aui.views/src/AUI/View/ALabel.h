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

#include <AUI/Enum/VerticalAlign.h>
#include <AUI/Enum/TextTransform.h>
#include "AView.h"
#include "AUI/Render/Render.h"
#include "AUI/Common/AString.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Enum/WordBreak.h"
#include <AUI/Util/ADataBinding.h>
#include <AUI/Common/IStringable.h>

/**
 * Represents a simple text.
 * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/ALabel.png">
 */
class API_AUI_VIEWS ALabel: public AView, public IStringable
{
private:
	AString mText;
	_<IDrawable> mIcon;
    _<AFont> mFontOverride;
    uint8_t mFontSizeOverride = 0;
    VerticalAlign mVerticalAlign = VerticalAlign::DEFAULT;
    TextTransform mTextTransform = TextTransform::NONE;
    AColor mIconColor = {1, 1, 1, 1};

	glm::ivec2 getIconSize() const;
    AString getTransformedText();
protected:
    Render::PrerenderedString mPrerendered;

	AFontStyle getFontStyleLabel();

	const Render::PrerenderedString& getPrerendered() {
	    return mPrerendered;
	}

    //void userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) override;


    // for correct selection positioning (used in ASelectableLabel)
    int mTextLeftOffset = 0;

public:
	ALabel();
	explicit ALabel(const AString& text);

	void render() override;
    void doRenderText();

	int getContentMinimumWidth() override;
	int getContentMinimumHeight() override;

    void invalidateFont() override;

    const _<IDrawable>& getIcon() const
	{
		return mIcon;
	}

    AString toString() const override;

    void setIcon(const _<IDrawable>& drawable) {
        mIcon = drawable;
        redraw();
    }

    void setIconColor(const AColor& iconColor) {
        mIconColor = iconColor;
    }

    void doPrerender();

    bool consumesClick(const glm::ivec2& pos) override;

    void onDpiChanged() override;

    void setText(const AString& newText);

	[[nodiscard]] const AString& getText() const
	{
		return mText;
	}

	void setFont(_<AFont> font) {
	    mFontOverride = std::move(font);
        invalidateFont();
	}
	void setFontSize(uint8_t size) {
        mFontSizeOverride = size;
        invalidateFont();
    }

    void setVerticalAlign(VerticalAlign verticalAlign) {
        mVerticalAlign = verticalAlign;
        invalidateFont();
	}
    void setTextTransform(TextTransform textTransform) {
        mTextTransform = textTransform;
        invalidateFont();
	}

    void setSize(int width, int height) override;
};


template<>
struct ADataBindingDefault<ALabel, AString> {
public:
    static void setup(const _<ALabel>& view) {}
    static auto getGetter() {
        return (ASignal<AString> ALabel::*)nullptr;
    }
    static auto getSetter() {
        return &ALabel::setText;
    }
};
