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

AAbstractTextField::~AAbstractTextField() {
}

bool AAbstractTextField::handlesNonMouseNavigation() {
    return true;
}


int AAbstractTextField::getContentMinimumHeight(ALayoutDirection layout) {
    return getFontStyle().size;
}


void AAbstractTextField::render(ARenderContext ctx) {
    AView::render(ctx);

    prerenderStringIfNeeded(ctx.render);

    AStaticVector<ARect<int>, 1> selectionRects;
    if (hasSelection()) {
        auto s = selection();
        auto beginPos = getPosByIndex(s.begin).x;
        auto endPos = getPosByIndex(s.end).x;
        selectionRects.push_back(ARect<int>::fromTopLeftPositionAndSize({mPadding.left + beginPos, mPadding.top + getVerticalAlignmentOffset()},
                                                                        {endPos - beginPos, getFontStyle().size}));
    }
    drawSelectionBeforeAndAfter(ctx.render, selectionRects, [&] {
        doDrawString(ctx.render);
    });
    drawCursor(ctx.render, {mAbsoluteCursorPos + mPadding.left, mPadding.top + getVerticalAlignmentOffset()});
}

void AAbstractTextField::doDrawString(IRenderer& render) {
    if (!mPrerenderedString) {
        return;
    }
    RenderHints::PushState m(render);
    render.translate(
            {mPadding.left - mHorizontalScroll + mTextAlignOffset, mPadding.top + getVerticalAlignmentOffset()});
    render.setColor(getTextColor());
    mPrerenderedString->draw();
}


void AAbstractTextField::setText(const AString& t) {
    mHorizontalScroll = 0;
    mContents = t;
    if (t.empty()) {
        clearSelection();
    }

    mCursorIndex = t.size();
    updateCursorBlinking();

    invalidateFont();
    emit textChanged(t);
}

void AAbstractTextField::setSuffix(const AString& s) {
    mSuffix = s;
    invalidateFont();
}

bool AAbstractTextField::wantsTouchscreenKeyboard() {
    return true;
}

AString AAbstractTextField::getDisplayText() {
    if (mIsPasswordTextField) {
        return AString(mContents.length(), L'•');
    }
    return mContents;
}

void AAbstractTextField::cursorSelectableRedraw() {
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
    if (!isValidText(mContents)) {
        mContents = std::move(contentsCopy);
        mCursorIndex = cursorIndexCopy;
        ADesktop::playSystemSound(ADesktop::SystemSound::ASTERISK);
    }
    emit textChanging(mContents);
}

void AAbstractTextField::prerenderStringIfNeeded(IRenderer& render) {
    if (!mPrerenderedString) {
        auto text = getDisplayText() + mSuffix;
        updateTextAlignOffset();
        if (!text.empty()) {
            auto canvas = render.newMultiStringCanvas(getFontStyle());
            canvas->enableCachingForTextLayoutHelper();
            switch (getFontStyle().align) {
                case ATextAlign::LEFT:
                    canvas->addString({0, 0}, text);
                    break;
                case ATextAlign::CENTER:
                    canvas->addString({0, 0}, text);
                    break;

                case ATextAlign::RIGHT:
                    canvas->addString({0, 0}, text);
                    break;

                case ATextAlign::JUSTIFY:
                    // justify cannot be handled here
                    break;
            }
            mTextLayoutHelper = canvas->getTextLayoutHelper();
            mPrerenderedString = canvas->finalize();
        } else {
            mTextLayoutHelper = {};
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

    auto w = getPosByIndex(text().length()).x;
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

unsigned AAbstractTextField::cursorIndexByPos(glm::ivec2 pos) {
    return mTextLayoutHelper.posToIndexFixedLineHeight(pos - glm::ivec2{ mPadding.left - mHorizontalScroll, 0 },
                                                       getFontStyle());
}

glm::ivec2 AAbstractTextField::getPosByIndex(int i) {
    int x = [&] {
        if (auto r = mTextLayoutHelper.indexToPos(0, i)) [[unlikely]] {
            return r->x;
        }
        // fallback as a slower implementation.
        return int(getFontStyle().getWidth(getDisplayText().substr(0, i)));
    }();
    return { -mHorizontalScroll + x, 0 };
}

void AAbstractTextField::onCursorIndexChanged() {
    auto absoluteCursorPos = getPosByIndex(mCursorIndex).x;

    if (absoluteCursorPos < 0)
    {
        mHorizontalScroll += absoluteCursorPos;
        absoluteCursorPos = 0;
    }
    else if (absoluteCursorPos >= this->getContentWidth())
    {
        mHorizontalScroll += absoluteCursorPos - this->getContentWidth();
        absoluteCursorPos = this->getContentWidth() - 1 /* -1 for cursor width */;
    } else if (hasSelection() && selection().end == mCursorIndex) {
        // apply -1 offset to cursor if cursor stays at the end of selection so it covers selection.
        absoluteCursorPos -= 1;
    }
    mAbsoluteCursorPos = absoluteCursorPos;
    mHorizontalScroll = glm::clamp(mHorizontalScroll, 0, glm::max(int(getPosByIndex(text().length()).x - this->getContentWidth()) + mHorizontalScroll, 0));
}
