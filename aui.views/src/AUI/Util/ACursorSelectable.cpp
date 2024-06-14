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

//
// Created by alex2 on 30.11.2020.
//

#include <AUI/Platform/AInput.h>
#include "AUI/Render/ARender.h"
#include <AUI/Render/RenderHints.h>
#include "ACursorSelectable.h"

ACursorSelectable::Selection ACursorSelectable::selection() const {
    return { glm::min(mCursorIndex, unsigned(mCursorSelection)), glm::max(mCursorIndex, unsigned(mCursorSelection)) };
}

bool ACursorSelectable::hasSelection() const {
    return mCursorIndex != mCursorSelection && mCursorSelection != -1;
}

unsigned ACursorSelectable::cursorIndexByPos(glm::ivec2 pos) {
    return mTextLayoutHelper.posToIndexFixedLineHeight(pos - getMouseSelectionPadding() + getMouseSelectionScroll(),
                                                       getMouseSelectionFont());
}

void ACursorSelectable::handleMousePressed(const APointerPressedEvent& event) {
    if (event.pointerIndex != APointerIndex::button(AInput::RBUTTON)) {
        mCursorSelection = mCursorIndex = cursorIndexByPos(event.position);
    }
}

void ACursorSelectable::handleMouseMove(const glm::ivec2& pos) {
    if (!mIgnoreSelection && isLButtonPressed()) {
        mCursorIndex = cursorIndexByPos(pos);
        onSelectionChanged();
        doRedraw();
    }
}

void ACursorSelectable::handleMouseReleased(const APointerReleasedEvent& event) {
    if (event.pointerIndex != APointerIndex::button(AInput::RBUTTON))
    {
        mIgnoreSelection = false;
        if (mCursorSelection == mCursorIndex)
        {
            mCursorSelection = -1;
        }
        onSelectionChanged();
    }
}

int ACursorSelectable::drawSelectionPre() {
    auto absoluteCursorPos = getPosByIndex(mCursorIndex);
    
    // selection
    if (hasSelection())
    {
        auto absoluteSelectionPos = getPosByIndex(mCursorSelection);

        mAbsoluteBegin = mCursorIndex < mCursorSelection ? absoluteCursorPos : absoluteSelectionPos;
        mAbsoluteEnd = mCursorIndex < mCursorSelection ? absoluteSelectionPos : absoluteCursorPos;

        RenderHints::PushColor c;
        ARender::setColor(AColor(1.f) - AColor(0x0078d700u));
        
        auto padding = getMouseSelectionPadding();
        drawSelectionRects();
    }
    return absoluteCursorPos;
}

int ACursorSelectable::getPosByIndex(int end, int begin) {
    return -getMouseSelectionScroll().x + int(getMouseSelectionFont().getWidth(getDisplayText().substr(begin, end - begin)));
}

void ACursorSelectable::drawSelectionPost() {
    ARender::setBlending(Blending::INVERSE_DST);
    if (hasSelection())
    {
        drawSelectionRects();
    }
}

void ACursorSelectable::selectAll() {
    size_t length = textLength();
    if (length > 0) {
        mCursorSelection = 0;
        mCursorIndex = length;
        onSelectionChanged();
    }
}

void ACursorSelectable::clearSelection() {
    mCursorSelection = -1;
}


void ACursorSelectable::drawSelectionRects() {
    auto p = getMouseSelectionPadding();

    int absoluteBeginPos = mAbsoluteBegin;
    int absoluteEndPos = mAbsoluteEnd;

    int row = 0;

    auto draw = [&]() {
        auto fs = getMouseSelectionFont();
        ARender::rect(ASolidBrush{},
                      {p.x + absoluteBeginPos, p.y + row * fs.getLineHeight() - 1},
                      {absoluteEndPos - absoluteBeginPos + 1, getMouseSelectionFont().size + 2});
    };

    auto t = getDisplayText();
    auto sel = selection();
    size_t lineBeginIndex = 0;
    for (size_t i = 0; i != sel.begin; ++i) {
        if (t[i] == '\n') {
            ++row;
        }
    }
    for (size_t i = sel.begin; i != sel.end; ++i) {
        if (t[i] == '\n') {
            absoluteEndPos = getPosByIndex(i, lineBeginIndex);
            draw();
            absoluteBeginPos = -getMouseSelectionScroll().x;
            lineBeginIndex = i;
            ++row;
        }
    }

    absoluteEndPos = getPosByIndex(sel.end, lineBeginIndex);
    draw();
}

void ACursorSelectable::handleMouseDoubleClicked(const APointerPressedEvent& event) {
    mIgnoreSelection = true;
    auto text = getDisplayText();

    // select word
    auto clickIndex = cursorIndexByPos(event.position);


    // determine the begin and end indices of the word
    auto end = text.find(' ', clickIndex);
    auto begin = text.rfind(' ', clickIndex);

    if (end == AString::NPOS) {
        end = text.length();
    }
    if (begin == AString::NPOS) {
        begin = 0;
    } else {
        begin += 1;
    }

    mCursorSelection = begin;
    mCursorIndex = end;
    onSelectionChanged();
}

