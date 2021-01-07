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

#include "ALabel.h"
#include <AUI/Render/RenderHints.h>
#include <glm/gtc/matrix_transform.hpp>

#include "AViewContainer.h"

#include <AUI/Platform/AWindow.h>
#include <AUI/Util/kAUI.h>

void ALabel::updateMultiline()
{
	mLines = getFontStyleLabel().font->trimStringToMultiline(mText, getWidth(), getFontStyleLabel().size, getFontStyleLabel().fontRendering);
	mPrerendered.mVao = nullptr;
}

ALabel::ALabel()
{

}

ALabel::ALabel(const AString& text):
	mText(text)
{

}

void ALabel::render()
{
	AView::render();

	doRenderText();
}


int ALabel::getContentMinimumWidth()
{
	if (mMultiline)
		return 10;
	if (!mPrerendered.mVao) {
	    doPrerender();
	}
    return mPrerendered.length;
	/*
	if (mPrerendered.mVao) {
	    return mPrerendered.length;
	}
	return getFontStyleLabel().getWidth(mText);*/
}

int ALabel::getContentMinimumHeight()
{
	if (mText.empty())
		return 0;
	if (mMultiline)
	{
		return mLines.size() * (getFontStyleLabel().font->getAscenderHeight(getFontStyleLabel().size)
		* (1.f + getFontStyleLabel().lineSpacing)) + getFontStyleLabel().font->getDescenderHeight(getFontStyleLabel().size) + 1;
	}
	return getFontStyleLabel().size;
}

void ALabel::setText(const AString& newText)
{
	mText = newText;
	mPrerendered.mVao = nullptr;
	if (mMultiline)
	{
		updateMultiline();
	}
    if (getParent())
        getParent()->updateLayout();
	redraw();
}

void ALabel::setMultiline(const bool multiline)
{
	mMultiline = multiline;
	mPrerendered.mVao = nullptr;
	if (multiline) {
		if (getParent())
			getParent()->updateLayout();
		//updateMultiline();
	}
	else
		mLines.clear();
	redraw();
}

void ALabel::setSize(int width, int height) {
    auto oldWidth = getWidth();
    AView::setSize(width, height);

    bool refresh = mMultiline && (mLines.empty() || oldWidth != getWidth());

    if (mMultiline) {
        updateMultiline();
        if (mLines.empty())
            return;
    }

    if (refresh) {
        AThread::current()->enqueue([&]()
        {
            if (getParent())
                getParent()->updateLayout();
        });
    }
}

FontStyle ALabel::getFontStyleLabel() {
    auto fs = getFontStyle();
    if (mFontOverride)
        fs.font = mFontOverride;
    if (mFontSizeOverride)
        fs.size = mFontSizeOverride;
    return fs;
}

/*
void ALabel::userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) {
    mPrerendered.mVao = nullptr;
    mVerticalAlign = Align::LEFT;
    processor(css::T_VERTICAL_ALIGN, [&](property p)
    {
        if (p->getArgs().size() == 1) {
            if (p->getArgs()[0] == "middle")
                mVerticalAlign = Align::CENTER;
            else
                mVerticalAlign = Align::LEFT;
        }
    });
    mTextTransform = TT_NORMAL;
    processor(css::T_TEXT_TRANSFORM, [&](property p)
    {
        if (p->getArgs().size() == 1) {
            if (p->getArgs()[0] == "uppercase")
                mTextTransform = TT_UPPERCASE;
            else if (p->getArgs()[0] == "lowercase")
                mTextTransform = TT_LOWERCASE;
        }
    });
}*/

void ALabel::doPrerender() {
    auto fs = getFontStyleLabel();
    auto t = getTargetText();
    if (t.empty())
        return;
    mPrerendered = Render::inst().preRendererString(t, fs);
}

void ALabel::doRenderText() {
    if (mMultiline && mLines.empty()) {
        updateMultiline();
    }
    if (!mPrerendered.mVao && !mText.empty())
    {
        doPrerender();
    }


    if (mPrerendered.mVao || mIcon)
    {
        mTextLeftOffset = 0;
        switch (getFontStyleLabel().align)
        {
            case TextAlign::LEFT:
                if (mIcon)
                {
                    auto requiredSpace = mIcon->getSizeHint();
                    RenderHints::PushState s;
                    Render::inst().setTransform(glm::translate(glm::mat4(1.f),
                                                               glm::vec3(mTextLeftOffset, (getContentHeight() - requiredSpace.y) / 2, 0)));
                    mIcon->draw(requiredSpace);
                    mTextLeftOffset += requiredSpace.x + 1;
                }
                break;

            case TextAlign::CENTER:
                mTextLeftOffset += getContentWidth() / 2;
                if (mIcon)
                {
                    auto requiredSpace = mIcon->getSizeHint();
                    RenderHints::PushState s;
                    Render::inst().setTransform(glm::translate(glm::mat4(1.f),
                                                               glm::vec3(mTextLeftOffset - (mPrerendered.mVao ? mPrerendered.fs.getWidth(mText) : 0) - requiredSpace.x / 2,
                                                                             (getContentHeight() - requiredSpace.y) / 2, 0)));
                    mIcon->draw(requiredSpace);
                }

                break;

            case TextAlign::RIGHT:
                mTextLeftOffset += getContentWidth();
                if (mIcon)
                {
                    auto requiredSpace = mIcon->getSizeHint();
                    RenderHints::PushState s;
                    Render::inst().setTransform(glm::translate(glm::mat4(1.f),
                                                               glm::vec3(mTextLeftOffset - (mPrerendered.mVao ? mPrerendered.fs.getWidth(mText) : 0) - requiredSpace.x / 2,
                                                                             (getContentHeight() - requiredSpace.y) / 2, 0)));
                    mIcon->draw(requiredSpace);
                }

                break;
        }
        if (mPrerendered.mVao) {
            int y = mPadding.top - (getFontStyleLabel().font->getDescenderHeight(getFontStyleLabel().size)) + 1;
            if (mVerticalAlign == VerticalAlign::MIDDLE) {
                if (mMultiline) {
                    y = (glm::max)(y, (getHeight() - getMinimumHeight()) / 2 - 1);
                } else {
                    y = (glm::max)(y, (getHeight() - getFontStyleLabel().size) / 2 - 1);
                }
            }
            Render::inst().drawString(mTextLeftOffset + mPadding.left, y, mPrerendered);
        }
    }
}

AString ALabel::getTargetText() {
    AString targetString;
    if (mMultiline) {
        targetString = mLines.join('\n');
    } else
    {
        targetString = mText;
    }
    if (targetString.empty())
        return {};
    switch (mTextTransform) {
        case TT_UPPERCASE:
            targetString = targetString.uppercase();
            break;
        case TT_LOWERCASE:
            targetString = targetString.lowercase();
            break;
    }
    return targetString;
}

void ALabel::onDpiChanged() {
    AView::onDpiChanged();
    uiX [&] {
        mPrerendered.mVao = nullptr;
        redraw();
    };
}

void ALabel::invalidateFont() {
    mPrerendered.mVao = nullptr;
}