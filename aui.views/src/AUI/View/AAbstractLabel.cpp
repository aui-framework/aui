
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


void AAbstractLabel::render(ClipOptimizationContext context)
{
	AView::render(context);

	doRenderText();
}


int AAbstractLabel::getContentMinimumWidth(ALayoutDirection layout)
{
    if (mTextOverflow != ATextOverflow::NONE)
        return 0;

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
{	if (mText.empty())
		return 0;

    return getFontStyleLabel().size;
}



void AAbstractLabel::setSize(glm::ivec2 size) {
    auto oldWidth = getWidth();
    AView::setSize(size);
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

template < class Iterator >
size_t AAbstractLabel::findFirstOverflowedIndex(const Iterator& begin,
                                             const Iterator& end,
                                             int overflowingWidth) {
    size_t gotWidth = 0;
    for (Iterator it = begin; it != end; ++it) {
        gotWidth += getFontStyleLabel().getWidth(it, it + 1);
        if (gotWidth <= overflowingWidth)
            continue;

        return it - begin;
    }

    return end - begin;
}

template < class Iterator >
void AAbstractLabel::processTextOverflow(Iterator begin, Iterator end, int overflowingWidth) {
    size_t firstOverflowedIndex = findFirstOverflowedIndex(begin, end, overflowingWidth);
    if (mTextOverflow == ATextOverflow::ELLIPSIS) {
        if (firstOverflowedIndex >= 3) {
            std::fill(begin + firstOverflowedIndex - 3, begin + firstOverflowedIndex, '.');
        } else {
            std::fill(begin, end, ' ');
        }
    }

    std::fill(begin + firstOverflowedIndex, end, ' ');
}

void AAbstractLabel::processTextOverflow(AString& text) {
    if (mTextOverflow == ATextOverflow::NONE)
        return;

    int overflowingWidth;
    if (getFixedSize().x == 0) {
        overflowingWidth = getMaxSize().x;
    } else {
        overflowingWidth = std::min(getMaxSize().x, getFixedSize().x);
    }

    overflowingWidth = std::min(overflowingWidth, mSize.x);

    mIsTextTooLarge = getFontStyleLabel().getWidth(text) > overflowingWidth;
    if (!mIsTextTooLarge)
        return;

    processTextOverflow(text.begin(), text.end(), overflowingWidth);
}

void AAbstractLabel::doPrerender() {
    auto fs = getFontStyleLabel();
    if (!mText.empty()) {
        AString transformedText = getTransformedText();
        processTextOverflow(transformedText);
        mPrerendered = ARender::prerenderString({0, 0}, transformedText, fs);
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
        auto alignLeft = [&] {
            if (mIcon) {
                requiredSpace *= getHeight() / requiredSpace.y;
                RenderHints::PushState s;
                ARender::setColor(mIconColor);
                ARender::setTransform(glm::translate(glm::mat4(1.f),
                                                     glm::vec3(mPadding.left + mTextLeftOffset, iconY, 0)));
                IDrawable::Params p;
                p.size = requiredSpace;
                mIcon->draw(p);
                mTextLeftOffset += requiredSpace.x + 4_dp;
            }
        };

        if (mIsTextTooLarge) {
            alignLeft();
        } else {
            switch (getFontStyleLabel().align) {
                case ATextAlign::LEFT:
                    alignLeft();
                    break;
                case ATextAlign::CENTER:
                    mTextLeftOffset += (getContentWidth() - mPrerendered->getWidth()) / 2;
                    if (mIcon) {
                        mTextLeftOffset += requiredSpace.x / 2;
                        RenderHints::PushState s;
                        ARender::setColor(mIconColor);
                        ARender::setTransform(glm::translate(glm::mat4(1.f),
                                                             glm::vec3(mTextLeftOffset - (mPrerendered->getWidth()) / 2 -
                                                                      requiredSpace.x,
                                                                      iconY, 0)));

                        IDrawable::Params p;
                        p.size = requiredSpace;
                        mIcon->draw(p);
                    }

                    break;

                case ATextAlign::RIGHT:
                    mTextLeftOffset += getContentWidth() - mPrerendered->getWidth();
                    if (mIcon) {
                        RenderHints::PushState s;
                        ARender::setColor(mIconColor);
                        ARender::setTransform(glm::translate(glm::mat4(1.f),
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
        }

        if (mPrerendered) {
            int y = mPadding.top;

            if (mVerticalAlign == VerticalAlign::MIDDLE) {
                auto ascenderHeight = getFontStyleLabel().font->getAscenderHeight(getFontStyleLabel().size);
                auto descenderHeight = getFontStyleLabel().font->getDescenderHeight(getFontStyleLabel().size);
                y = (glm::max)(y, y + int(glm::ceil((getContentHeight() - int(ascenderHeight + descenderHeight)) / 2.0)));
            }
            RenderHints::PushMatrix m;
            ARender::translate({mTextLeftOffset + mPadding.left, y });
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

void AAbstractLabel::setText(AString newText) {
    AUI_ASSERT_UI_THREAD_ONLY();
    if (mText == newText) {
        return;
    }
    mText = std::move(newText);
    mPrerendered = nullptr;

    requestLayoutUpdate();
    redraw();
}
