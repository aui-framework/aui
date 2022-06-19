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

//
// Created by alex2 on 30.11.2020.
//

#include <AUI/Platform/AInput.h>
#include <AUI/Render/Render.h>
#include <AUI/Render/RenderHints.h>
#include "ACursorSelectable.h"

ACursorSelectable::Selection ACursorSelectable::getSelection() const {
    return { glm::min(mCursorIndex, unsigned(mCursorSelection)), glm::max(mCursorIndex, unsigned(mCursorSelection)) };
}

bool ACursorSelectable::hasSelection() const {
    return mCursorIndex != mCursorSelection && mCursorSelection != -1;
}

unsigned ACursorSelectable::getCursorIndexByPos(glm::ivec2 pos) {
    return mTextLayoutHelper.posToIndexFixedLineHeight(pos - getMouseSelectionPadding(), getMouseSelectionFont());
}

void ACursorSelectable::handleMousePressed(const glm::ivec2& pos, AInput::Key button) {
    if (button == AInput::LBUTTON) {
        mCursorSelection = mCursorIndex = getCursorIndexByPos(pos);
    }
}

void ACursorSelectable::handleMouseMove(const glm::ivec2& pos) {
    if (!mIgnoreSelection && isLButtonPressed()) {
        mCursorIndex = getCursorIndexByPos(pos);
        doRedraw();
    }
}

void ACursorSelectable::handleMouseReleased(const glm::ivec2& pos, AInput::Key button) {
    if (button == AInput::LBUTTON)
    {
        mIgnoreSelection = false;
        if (mCursorSelection == mCursorIndex)
        {
            mCursorSelection = -1;
        }
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
        Render::setColor(AColor(1.f) - AColor(0x0078d700u));
        
        auto padding = getMouseSelectionPadding();
        drawSelectionRects();
    }
    return absoluteCursorPos;
}

int ACursorSelectable::getPosByIndex(int end, int begin) {
    return -getMouseSelectionScroll().x + int(getMouseSelectionFont().getWidth(getDisplayText().mid(begin, end - begin)));
}

void ACursorSelectable::drawSelectionPost() {
    Render::setBlending(Blending::INVERSE_DST);
    if (hasSelection())
    {
        drawSelectionRects();
    }
}

void ACursorSelectable::selectAll() {
    size_t length = getTextLength();
    if (length > 0) {
        mCursorSelection = 0;
        mCursorIndex = length;
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
        Render::rect(ASolidBrush{},
                     {p.x + absoluteBeginPos, p.y + row * fs.getLineHeight()},
                     {absoluteEndPos - absoluteBeginPos + 1, getMouseSelectionFont().size + 3});
    };

    auto t = getDisplayText();
    auto sel = getSelection();
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

void ACursorSelectable::handleMouseDoubleClicked(const glm::ivec2& pos, AInput::Key button) {
    mIgnoreSelection = true;
    auto text = getDisplayText();

    // select word
    auto clickIndex = getCursorIndexByPos(pos);


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
}

