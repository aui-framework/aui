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

#include <AUI/Platform/AClipboard.h>
#include <AUI/Platform/ABaseWindow.h>
#include "AAbstractTextField.h"


#include "AUI/Platform/APlatform.h"
#include "AUI/Platform/ADesktop.h"
#include <AUI/Util/AMetric.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Render/RenderHints.h>


AAbstractTextField::AAbstractTextField() {
}

void AAbstractTextField::onFocusAcquired() {
    AView::onFocusAcquired();
    updateCursorBlinking();
}

void AAbstractTextField::onFocusLost() {
    AAbstractTypeableView::onFocusLost();
}

AAbstractTextField::~AAbstractTextField()
{
}

bool AAbstractTextField::handlesNonMouseNavigation() {
    return true;
}


int AAbstractTextField::getContentMinimumHeight(ALayoutDirection layout)
{
	return getFontStyle().size;
}


void AAbstractTextField::render(ARenderContext ctx)
{
	AView::render(ctx);

    prerenderStringIfNeeded(ctx.render);
	if (hasFocus()) {
	    auto absoluteCursorPos = ACursorSelectable::drawSelectionPre(ctx.render);

	    // text
        doDrawString(ctx.render);

        ACursorSelectable::drawSelectionPost(ctx.render);

        // cursor
        if (hasFocus() && isCursorBlinkVisible()) {
            if (absoluteCursorPos < 0) {
                mHorizontalScroll += absoluteCursorPos;
                redraw();
            } else if (getWidth() < absoluteCursorPos + mPadding.horizontal() + 1) {
                mHorizontalScroll += absoluteCursorPos - getWidth() + mPadding.horizontal() + 1;
                redraw();
            }

            auto p = getMouseSelectionPadding();
            ctx.render.rectangle(ASolidBrush{},
                                 {p.x + absoluteCursorPos, p.y - 1},
                                 {1, getMouseSelectionFont().size + 2});
        }

        ctx.render.setBlending(Blending::NORMAL);
    } else {
        doDrawString(ctx.render);
	}

}

glm::ivec2 AAbstractTextField::getMouseSelectionPadding() {
    auto p = AAbstractTypeableView::getMouseSelectionPadding();
    p.x += mTextAlignOffset;
    return p;
}

void AAbstractTextField::doDrawString(IRenderer& render) {
    RenderHints::PushMatrix m(render);
    render.translate({ mPadding.left - mHorizontalScroll + mTextAlignOffset, mPadding.top + getVerticalAlignmentOffset() });
    if (mPrerenderedString) mPrerenderedString->draw();
}


void AAbstractTextField::setText(const AString& t)
{
    mHorizontalScroll = 0;
	mContents = t;
    if (t.empty()) {
        clearSelection();
    }

    mCursorIndex = t.size();
	updateCursorBlinking();

    invalidatePrerenderedString();
	emit textChanged(t);
}

bool AAbstractTextField::wantsTouchscreenKeyboard() {
    return true;
}

AString AAbstractTextField::getContentsPasswordWrap() {
    if (mIsPasswordTextField) {
        return AString(mContents.length(), L'•');
    }
    return mContents;
}

AString AAbstractTextField::getDisplayText() {
    return getContentsPasswordWrap();
}

void AAbstractTextField::doRedraw() {
    redraw();
}

const AString& AAbstractTextField::text() const {
    return mContents;
}

void AAbstractTextField::typeableErase(size_t begin, size_t end) {
    if (begin >= mContents.length()) {
        return;
    }
    mContents.erase(mContents.begin() + begin, mContents.begin() + end);
}

bool AAbstractTextField::typeableInsert(size_t at, const AString& toInsert) {
    mContents.insert(at, toInsert);
    if (!isValidText(mContents)) {
        mContents.erase(at, toInsert.length()); // undo insert
        return false;
    }
    return true;
}

bool AAbstractTextField::typeableInsert(size_t at, char16_t toInsert) {
    mContents.insert(at, toInsert);
    if (!isValidText(mContents)) {
        mContents.erase(at, 1); // undo insert
        return false;
    }
    return true;
}

size_t AAbstractTextField::typeableFind(char16_t c, size_t startPos) {
    return mContents.find(c, startPos);
}

size_t AAbstractTextField::typeableReverseFind(char16_t c, size_t startPos) {
    return mContents.rfind(c, startPos);
}

size_t AAbstractTextField::length() const {
    return mContents.length();
}

void AAbstractTextField::invalidatePrerenderedString() {
    mPrerenderedString = nullptr;
}

void AAbstractTextField::invalidateFont() {
    mPrerenderedString = nullptr;
}

void AAbstractTextField::onCharEntered(char16_t c) {
    mCursorIndex = std::min(mCursorIndex, static_cast<unsigned int> (mContents.size()));
    if (c == '\n' || c == '\r')
        return;
    AView::onCharEntered(c);
    auto contentsCopy = mContents;
    auto cursorIndexCopy = mCursorIndex;

    enterChar(c);
    if (!isValidText(mContents))
    {
        mContents = std::move(contentsCopy);
        mCursorIndex = cursorIndexCopy;
        ADesktop::playSystemSound(ADesktop::SystemSound::ASTERISK);
    }
    emit textChanging(mContents);
}

void AAbstractTextField::prerenderStringIfNeeded(IRenderer& render) {
    if (!mPrerenderedString) {
        auto text = getContentsPasswordWrap();
        updateTextAlignOffset();
        if (!text.empty()) {
            auto canvas = render.newMultiStringCanvas(getFontStyle());
            canvas->enableCachingForTextLayoutHelper();
            switch (getFontStyle().align) {
                case ATextAlign::LEFT:
                    canvas->addString({ 0, 0 }, text);
                    break;
                case ATextAlign::CENTER:
                    canvas->addString({ 0, 0 }, text);
                    break;

                case ATextAlign::RIGHT:
                    canvas->addString({ 0, 0 }, text);
                    break;

                case ATextAlign::JUSTIFY:
                    // justify cannot be handled here
                    break;
            }
            setTextLayoutHelper(canvas->getTextLayoutHelper());
            mPrerenderedString = canvas->finalize();
        } else {
            setTextLayoutHelper({});
        }
    }
}

void AAbstractTextField::updateTextAlignOffset() {
    switch (getFontStyle().align) {
        case ATextAlign::JUSTIFY:
        case ATextAlign::LEFT:
            mTextAlignOffset = 0;
            return;
        default:
            break;
    }

    auto w = getFontStyle().getWidth(getContentsPasswordWrap());
    if (w >= getContentWidth()) {
        mTextAlignOffset = 0; // unbreak the scroll
        return;
    }

    switch (getFontStyle().align) {
        case ATextAlign::CENTER:
            mTextAlignOffset = (getContentWidth() - w) / 2;
            return;

        case ATextAlign::RIGHT:
            mTextAlignOffset = getContentWidth() - w;
            return;

        default:
            break;
    }
}

bool AAbstractTextField::isValidText(const AString& text) {
    return true;
}

size_t AAbstractTextField::textLength() const {
    return mContents.length();
}

AString AAbstractTextField::toString() const {
    return mContents;
}

void AAbstractTextField::setSize(glm::ivec2 size) {
    AView::setSize(size);
    updateTextAlignOffset();
}
void AAbstractTextField::onKeyDown(AInput::Key key) {
    AAbstractTypeableView::onKeyDown(key);
    if (key == AInput::Key::RETURN) {
        emit actionButtonPressed;
    }
}
