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

#include <AUI/Platform/AClipboard.h>
#include <AUI/Platform/AWindowBase.h>
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


int AAbstractTextField::getContentMinimumHeight() {
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
    if (!mIsEditable) {
        return;
    }
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
    auto utf32t = t.encode(AStringEncoding::UTF32);
    mContents = {reinterpret_cast<const char32_t*>(utf32t.data()), utf32t.size() / sizeof(char32_t)};
    if (t.empty()) {
        clearSelection();
    }

    mCursorIndex = utf32t.size() / sizeof(char32_t);
    onCursorIndexChanged();
    updateCursorBlinking();
    invalidateFont();
    emit textChanged(t);
}

void AAbstractTextField::setSuffix(const AString& s) {
    mSuffix = s;
    invalidateFont();
}

std::u32string AAbstractTextField::getDisplayText() {
    if (mIsPasswordTextField) {
        return std::u32string(mContents.length(), U'•');
    }
    return mContents;
}

void AAbstractTextField::cursorSelectableRedraw() {
    redraw();
}

AString AAbstractTextField::getText() const {
    return AString(mContents);
}

void AAbstractTextField::typeableErase(size_t begin, size_t end) {
    if (!mIsEditable) {
        return;
    }
    if (begin >= mContents.length()) {
        return;
    }
    mContents.erase(mContents.begin() + begin, mContents.begin() + end);
}

bool AAbstractTextField::typeableInsert(size_t at, const AString& toInsert) {
    if (!mIsEditable) {
        return false;
    }
    auto u32str = toInsert.encode(AStringEncoding::UTF32);
    mContents.insert(at, reinterpret_cast<const char32_t*>(u32str.data()), u32str.size() / sizeof(char32_t));
    if (!isValidText(mContents)) {
        mContents.erase(at, u32str.size() / sizeof(char32_t)); // undo insert
        return false;
    }
    return true;
}

bool AAbstractTextField::typeableInsert(size_t at, AChar toInsert) {
    if (!mIsEditable) {
        return false;
    }
    mContents.insert(at, 1, toInsert);
    if (!isValidText(mContents)) {
        mContents.erase(at, 1); // undo insert
        return false;
    }
    return true;
}

size_t AAbstractTextField::typeableFind(AChar c, size_t startPos) {
    return mContents.find(c, startPos);
}

size_t AAbstractTextField::typeableReverseFind(AChar c, size_t startPos) {
    return mContents.rfind(c, startPos);
}

size_t AAbstractTextField::length() const {
    return mContents.length();
}

void AAbstractTextField::invalidateFont() {
    mPrerenderedString = nullptr;
}

void AAbstractTextField::onCharEntered(AChar c) {
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
    emit textChanging(AString(mContents));
}

void AAbstractTextField::prerenderStringIfNeeded(IRenderer& render) {
    if (!mPrerenderedString) {
        auto text = getDisplayText() + mSuffix.toUtf32();
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

    auto w = getPosByIndexAbsolute(getDisplayText().length());
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
void AAbstractTextField::commitStyle() {
    AAbstractTypeableView::commitStyle();
    onCursorIndexChanged();
}

bool AAbstractTextField::isValidText(std::u32string_view text) {
    return true;
}

AString AAbstractTextField::toString() const {
    return AString(mContents);
}

void AAbstractTextField::setSize(glm::ivec2 size) {
    AView::setSize(size);
    onCursorIndexChanged(); // cursor and horizontal scroll should respond to size changes.
}

unsigned AAbstractTextField::cursorIndexByPos(glm::ivec2 pos) {
    return mTextLayoutHelper.posToIndexFixedLineHeight(glm::ivec2{ pos.x - mPadding.left + mHorizontalScroll - mTextAlignOffset, 0 },
                                                       getFontStyle());
}

glm::ivec2 AAbstractTextField::getPosByIndex(size_t index) {
    int x = getPosByIndexAbsolute(index);
    return { -mHorizontalScroll + x + mTextAlignOffset, 0 };
}

int AAbstractTextField::getPosByIndexAbsolute(size_t index) {
    if (auto r = mTextLayoutHelper.indexToPos(0, index)) [[unlikely]] {
        return r->x;
    }
    // fallback as a slower implementation.
    return int(getFontStyle().getWidth(getDisplayText().substr(0, index)));
}

void AAbstractTextField::onCursorIndexChanged() {
    updateTextAlignOffset();
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
    auto horizontalScroll = glm::clamp(mHorizontalScroll, 0, glm::max(int(getPosByIndex(getText().length()).x - this->getContentWidth()) + mHorizontalScroll, 0));
    if (horizontalScroll != mHorizontalScroll) {
        mAbsoluteCursorPos = mHorizontalScroll - horizontalScroll;
        mHorizontalScroll = horizontalScroll;
    }
}

glm::ivec2 AAbstractTextField::getCursorPosition() {
    return {mAbsoluteCursorPos, 0};
}
