
/*
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

#include "AAbstractLabel.h"
#include <AUI/Render/RenderHints.h>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

#include "AViewContainer.h"

#include <AUI/Platform/AWindow.h>
#include <AUI/Util/kAUI.h>


AAbstractLabel::AAbstractLabel()
{

}


void AAbstractLabel::render()
{
	AView::render();

	doRenderText();
}


int AAbstractLabel::getContentMinimumWidth(ALayoutDirection layout)
{
	if (!mPrerendered) {
	    doPrerender();
	}
	int acc = mPrerendered ? mPrerendered->getWidth() : 0;
	if (mIcon) {
	    acc += getIconSize().x * 2;
	}
    return acc;
}

int AAbstractLabel::getContentMinimumHeight(ALayoutDirection layout)
{
	if (mText.empty())
		return 0;

    return getFontStyleLabel().size;
}



void AAbstractLabel::setSize(int width, int height) {
    auto oldWidth = getWidth();
    AView::setSize(width, height);
}

AFontStyle AAbstractLabel::getFontStyleLabel() {
    auto fs = getFontStyle();
    if (mFontOverride)
        fs.font = mFontOverride;
    if (mFontSizeOverride)
        fs.size = mFontSizeOverride;
    return fs;
}

/*
void AAbstractLabel::userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) {
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

void AAbstractLabel::doPrerender() {
    auto fs = getFontStyleLabel();
    if (!mText.empty()) {
        mPrerendered = Render::prerenderString({0, 0}, getTransformedText(), fs);

    }
}

void AAbstractLabel::doRenderText() {
    if (!mPrerendered)
    {
        doPrerender();
    }


    if (mPrerendered)
    {
        mTextLeftOffset = 0;
        auto requiredSpace = getIconSize();
        auto iconY = glm::ceil((getHeight() - requiredSpace.y) / 2.0);
        switch (getFontStyleLabel().align) {
            case TextAlign::LEFT:
                if (mIcon) {
                    requiredSpace *= getHeight() / requiredSpace.y;
                    RenderHints::PushState s;
                    Render::setColor(mIconColor);
                    Render::setTransform(glm::translate(glm::mat4(1.f),
                                                        glm::vec3(mPadding.left + mTextLeftOffset, iconY, 0)));
                    IDrawable::Params p;
                    p.size = requiredSpace;
                    mIcon->draw(p);
                    mTextLeftOffset += requiredSpace.x + 4_dp;
                }
                break;

            case TextAlign::CENTER:
                mTextLeftOffset += (getContentWidth() - mPrerendered->getWidth()) / 2;
                if (mIcon) {
                    mTextLeftOffset += requiredSpace.x / 2;
                    RenderHints::PushState s;
                    Render::setColor(mIconColor);
                    Render::setTransform(glm::translate(glm::mat4(1.f),
                                                        glm::vec3(mTextLeftOffset - (mPrerendered->getWidth()) / 2 -
                                                                  requiredSpace.x,
                                                                  iconY, 0)));

                    IDrawable::Params p;
                    p.size = requiredSpace;
                    mIcon->draw(p);
                }

                break;

            case TextAlign::RIGHT:
                mTextLeftOffset += getContentWidth() - mPrerendered->getWidth();
                if (mIcon) {
                    RenderHints::PushState s;
                    Render::setColor(mIconColor);
                    Render::setTransform(glm::translate(glm::mat4(1.f),
                                                        glm::vec3(mPadding.left + mTextLeftOffset -
                                                                  (mPrerendered ? mPrerendered->getWidth() : 0) -
                                                                  requiredSpace.x / 2,
                                                                  iconY, 0)));

                    IDrawable::Params p;
                    p.size = requiredSpace;
                    mIcon->draw(p);
                }

                break;
        }
        if (mPrerendered) {
            int y = mPadding.top;

            if (mVerticalAlign == VerticalAlign::MIDDLE) {
                auto ascenderHeight = getFontStyleLabel().font->getAscenderHeight(getFontStyleLabel().size);
                auto descenderHeight = getFontStyleLabel().font->getDescenderHeight(getFontStyleLabel().size);
                y = (glm::max)(y, y + int(glm::ceil((getContentHeight() - int(ascenderHeight + descenderHeight)) / 2.0)));
            }
            RenderHints::PushMatrix m;
            Render::translate({ mTextLeftOffset + mPadding.left, y });
            mPrerendered->draw();
        }
    }
}

AString AAbstractLabel::getTransformedText() {
    if (text().empty())
        return {};
    switch (mTextTransform) {
        case TextTransform::UPPERCASE:
            return text().uppercase();
        case TextTransform::LOWERCASE:
            return text().lowercase();
    }
    return text();
}

void AAbstractLabel::onDpiChanged() {
    AView::onDpiChanged();
    ui_threadX [&] {
        mPrerendered = nullptr;
        redraw();
    };
}

void AAbstractLabel::invalidateFont() {
    mPrerendered = nullptr;
    redraw();
}

glm::ivec2 AAbstractLabel::getIconSize() const {
    if (mIcon) {
        return {mIcon->getSizeHint().x * getContentHeight() / mIcon->getSizeHint().y, getContentHeight()};
    }
    return {};
}

bool AAbstractLabel::consumesClick(const glm::ivec2& pos) {
    return false;
}

AString AAbstractLabel::toString() const {
    return mText;
}

