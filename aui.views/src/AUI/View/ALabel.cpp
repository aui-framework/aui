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

#include "ALabel.h"
#include <AUI/Render/RenderHints.h>
#include <glm/gtc/matrix_transform.hpp>

#include "AViewContainer.h"

#include <AUI/Platform/AWindow.h>
#include <AUI/Util/kAUI.h>


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
	if (isMultiline())
		return 10;
	if (!mPrerendered) {
	    doPrerender();
	}
	int acc = mPrerendered->getWidth();
	if (mIcon) {
	    acc += getIconSize().x * 2;
	}
    return acc;
}

int ALabel::getContentMinimumHeight()
{
	if (mText.empty())
		return 0;
	if (mPrerendered && isMultiline())
	{
        return mPrerendered->getHeight();
	}
	return getFontStyleLabel().size;
}

void ALabel::setText(const AString& newText)
{
    // try to determine is text changed.

    auto sourceIterator = newText.begin();
    auto destinationIterator = mText.begin();

    if (newText.length() == mText.length()) {
        for (; sourceIterator != newText.end(); ++sourceIterator, ++destinationIterator) {
            if (*destinationIterator != *sourceIterator) {
                break;
            }
        }
        if (sourceIterator == newText.end()) {
            return;
        }
    } else {
        mText.resize(newText.size());
        destinationIterator = mText.begin();
    }
    for (; sourceIterator != newText.end(); ++sourceIterator, ++destinationIterator) {
        *destinationIterator = *sourceIterator;
    }

	mPrerendered = nullptr;

    if (getParent())
        getParent()->updateLayout();
	redraw();
}


void ALabel::setMultiline(const bool multiline)
{
    if (isMultiline() != multiline) {
        mPrerendered = nullptr;
        if (multiline) {
            mMultilineTextRender = std::make_unique<AMultilineTextRender>();
            if (getParent())
                getParent()->updateLayout();
        } else {
            mMultilineTextRender = nullptr;
        }
        redraw();
    }
}

void ALabel::setSize(int width, int height) {
    auto oldWidth = getWidth();
    AView::setSize(width, height);

    if (isMultiline() && oldWidth != getWidth()) {
        mPrerendered = nullptr;
        AThread::current()->enqueue([&]()
        {
            if (getParent())
                getParent()->updateLayout();
        });
    }
}

AFontStyle ALabel::getFontStyleLabel() {
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
    if (isMultiline()) {
        mMultilineTextRender->setFontStyle(getFontStyleLabel());
        mPrerendered = mMultilineTextRender->updateText(getTransformedText(), getSize());
    } else {
        mPrerendered = Render::prerenderString({0, 0}, getTransformedText(), fs);
    }
}

void ALabel::doRenderText() {
    if (!mPrerendered && !mText.empty())
    {
        doPrerender();
    }


    if (mPrerendered)
    {
        mTextLeftOffset = 0;
        if (isMultiline()) {
            switch (getFontStyleLabel().align) {
                case TextAlign::LEFT:
                    if (mIcon) {
                        auto requiredSpace = getIconSize();
                        requiredSpace *= getHeight() / requiredSpace.y;
                        RenderHints::PushState s;
                        Render::setColor(mIconColor);
                        Render::setTransform(glm::translate(glm::mat4(1.f),
                                                            glm::vec3(mPadding.left + mTextLeftOffset, 2_dp, 0)));
                        IDrawable::Params p;
                        p.size = requiredSpace;
                        mIcon->draw(p);
                        mTextLeftOffset += requiredSpace.x + 4_dp;
                    }
                    break;

                case TextAlign::CENTER:
                    mTextLeftOffset += (getContentWidth() - mPrerendered->getWidth()) / 2;
                    if (mIcon) {
                        auto requiredSpace = getIconSize();
                        mTextLeftOffset += requiredSpace.x / 2;
                        RenderHints::PushState s;
                        Render::setColor(mIconColor);
                        Render::setTransform(glm::translate(glm::mat4(1.f),
                                                            glm::vec3(mTextLeftOffset - (mPrerendered->getWidth()) / 2 -
                                                                      requiredSpace.x,
                                                                      (getHeight() - requiredSpace.y) / 2, 0)));

                        IDrawable::Params p;
                        p.size = requiredSpace;
                        mIcon->draw(p);
                    }

                    break;

                case TextAlign::RIGHT:
                    mTextLeftOffset += getContentWidth() - mPrerendered->getWidth();
                    if (mIcon) {
                        auto requiredSpace = getIconSize();
                        RenderHints::PushState s;
                        Render::setColor(mIconColor);
                        Render::setTransform(glm::translate(glm::mat4(1.f),
                                                            glm::vec3(mPadding.left + mTextLeftOffset -
                                                                      (mPrerendered ? mPrerendered->getWidth() : 0) -
                                                                      requiredSpace.x / 2,
                                                                      (getHeight() - requiredSpace.y) / 2, 0)));

                        IDrawable::Params p;
                        p.size = requiredSpace;
                        mIcon->draw(p);
                    }

                    break;
            }
        }
        if (mPrerendered) {
            int y = mPadding.top - (getFontStyleLabel().font->getDescenderHeight(getFontStyleLabel().size)) + 1;
            if (mVerticalAlign == VerticalAlign::MIDDLE) {
                if (isMultiline()) {
                    y = (glm::max)(y, (getHeight() - getMinimumHeight()) / 2 - 1);
                } else {
                    y = (glm::max)(y, int(getHeight() - getFontStyleLabel().size) / 2 - 1);
                }
            }
            RenderHints::PushMatrix m;
            Render::translate({ mTextLeftOffset + mPadding.left, y });
            mPrerendered->draw();
        }
    }
}

AString ALabel::getTransformedText() {
    if (getText().empty())
        return {};
    switch (mTextTransform) {
        case TextTransform::UPPERCASE:
            return getText().uppercase();
        case TextTransform::LOWERCASE:
            return getText().lowercase();
    }
    return getText();
}

void ALabel::onDpiChanged() {
    AView::onDpiChanged();
    ui_threadX [&] {
        mPrerendered = nullptr;
        redraw();
    };
}

void ALabel::invalidateFont() {
    mPrerendered = nullptr;
    redraw();
}

glm::ivec2 ALabel::getIconSize() const {
    if (mIcon) {
        return {mIcon->getSizeHint().x * getContentHeight() / mIcon->getSizeHint().y, getContentHeight()};
    }
    return {};
}

bool ALabel::consumesClick(const glm::ivec2& pos) {
    return false;
}

