/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
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


AAbstractLabel::AAbstractLabel() {

}


void AAbstractLabel::render(ARenderContext context) {
    AView::render(context);

    doRenderText(context.render);
}

int AAbstractLabel::getContentMinimumWidth() {
    if (mTextOverflow != ATextOverflow::NONE)
        return 0;

    int acc = mPrerendered ? mPrerendered->getWidth() : getFontStyle().getWidth(mText);
    if (mIcon) {
        acc += getIconSize().x * 2;
    }
    return acc;
}

int AAbstractLabel::getContentMinimumHeight() {
    if (mText.empty())
        return 0;

    return getFontStyle().size;
}


void AAbstractLabel::setSize(glm::ivec2 size) {
    AView::setSize(size);
    if (mTextOverflow != ATextOverflow::NONE) {
        mPrerendered = nullptr;
        redraw();
    }
}

template<class Iterator>
Iterator AAbstractLabel::findFirstOverflowedIndex(const Iterator& begin,
                                                  const Iterator& end,
                                                  int overflowingWidth) {
    size_t gotWidth = 0;
    for (Iterator it = begin; it != end; ++it) {
        gotWidth += getFontStyle().getWidth(it, it + 1);
        if (gotWidth <= overflowingWidth)
            continue;

        return it;
    }

    return end;
}

template<class Iterator>
void AAbstractLabel::processTextOverflow(Iterator begin, Iterator end, int overflowingWidth) {
    static constexpr auto ELLIPSIS = u'…';
    auto firstOverflowedIt = findFirstOverflowedIndex(
        begin, end, overflowingWidth - (mTextOverflow == ATextOverflow::ELLIPSIS ? getFontStyle().getWidth({ELLIPSIS}) : 0));
    if (firstOverflowedIt == end) {
        return;
    }
    if (mTextOverflow == ATextOverflow::ELLIPSIS) {
        *firstOverflowedIt = ELLIPSIS;
        firstOverflowedIt++;
    }

    std::fill(firstOverflowedIt, end, ' ');
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

    mIsTextTooLarge = getFontStyle().getWidth(text) > overflowingWidth;
    if (!mIsTextTooLarge)
        return;

    processTextOverflow(text.begin(), text.end(), overflowingWidth);
}

void AAbstractLabel::doPrerender(IRenderer& render) {
    auto fs = getFontStyle();
    if (!mText.empty()) {
        AString transformedText = getTransformedText();
        processTextOverflow(transformedText);
        mPrerendered = render.prerenderString({0, 0}, transformedText, fs);
    }
}

void AAbstractLabel::doRenderText(IRenderer& render) {
    if (!mPrerendered) {
        doPrerender(render);
    }


    if (mPrerendered) {
        mTextLeftOffset = 0;
        auto requiredSpace = getIconSize();
        auto iconY = glm::ceil((getHeight() - requiredSpace.y) / 2.0);
        auto alignLeft = [&] {
            if (mIcon) {
                requiredSpace *= getHeight() / requiredSpace.y;
                RenderHints::PushState s(render);
                render.setColor(mIconColor);
                render.setTransform(glm::translate(glm::mat4(1.f),
                                                   glm::vec3(mPadding.left + mTextLeftOffset, iconY, 0)));
                IDrawable::Params p;
                p.size = requiredSpace;
                mIcon->draw(render, p);
                mTextLeftOffset += requiredSpace.x + 4_dp;
            }
        };

        if (mIsTextTooLarge) {
            alignLeft();
        } else {
            switch (getFontStyle().align) {
                case ATextAlign::LEFT:
                    alignLeft();
                    break;
                case ATextAlign::CENTER:
                    mTextLeftOffset += (getContentWidth() - mPrerendered->getWidth()) / 2;
                    if (mIcon) {
                        mTextLeftOffset += requiredSpace.x / 2;
                        RenderHints::PushState s(render);
                        render.setColor(mIconColor);
                        render.setTransform(glm::translate(glm::mat4(1.f),
                                                           glm::vec3(mTextLeftOffset - (mPrerendered->getWidth()) / 2 -
                                                                     requiredSpace.x,
                                                                     iconY, 0)));

                        IDrawable::Params p;
                        p.size = requiredSpace;
                        mIcon->draw(render, p);
                    }

                    break;

                case ATextAlign::RIGHT:
                    mTextLeftOffset += getContentWidth() - mPrerendered->getWidth();
                    if (mIcon) {
                        RenderHints::PushState s(render);
                        render.setColor(mIconColor);
                        render.setTransform(glm::translate(glm::mat4(1.f),
                                                           glm::vec3(mPadding.left + mTextLeftOffset -
                                                                     (mPrerendered ? mPrerendered->getWidth() : 0) -
                                                                     requiredSpace.x / 2,
                                                                     iconY, 0)));

                        IDrawable::Params p;
                        p.size = requiredSpace;
                        mIcon->draw(render, p);
                    }

                    break;
                case ATextAlign::JUSTIFY:
                    break;
            }
        }

        if (mPrerendered) {
            int y = mPadding.top;

            if (mVerticalAlign == VerticalAlign::MIDDLE) {
                auto ascenderHeight = getFontStyle().font->getAscenderHeight(getFontStyle().size);
                auto descenderHeight = getFontStyle().font->getDescenderHeight(getFontStyle().size);
                y = (glm::max)(y,
                               y + int(glm::ceil((getContentHeight() - int(ascenderHeight + descenderHeight)) / 2.0)));
            }
            RenderHints::PushMatrix m(render);
            render.translate({mTextLeftOffset + mPadding.left, y});
            render.setColor(getTextColor());
            mPrerendered->draw();
        }
    }
}

AString AAbstractLabel::getTransformedText() {
    if (text()->empty())
        return {};
    switch (mTextTransform) {
        case TextTransform::UPPERCASE:
            return text()->uppercase();
        case TextTransform::LOWERCASE:
            return text()->lowercase();
        case TextTransform::NONE:
            break;
    }
    return text();
}

void AAbstractLabel::onDpiChanged() {
    AView::onDpiChanged();
    AUI_UI_THREAD_X [this, self = shared_from_this()] {
        mPrerendered = nullptr;
        redraw();
    };
}

void AAbstractLabel::invalidateFont() {
    mPrerendered = nullptr;
    markMinContentSizeInvalid();
    redraw();
}

glm::ivec2 AAbstractLabel::getIconSize() const {
    if (mIcon) {
        return {mIcon->getSizeHint().x * getContentHeight() / mIcon->getSizeHint().y, getContentHeight()};
    }
    return {};
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

    markMinContentSizeInvalid();
    redraw();

    emit mTextChanged(mText);
}

void AAbstractLabel::invalidateAllStyles() {
    invalidateAllStylesFont();
    AView::invalidateAllStyles();
}

void AAbstractLabel::commitStyle() {
    AView::commitStyle();
    commitStyleFont();
}
