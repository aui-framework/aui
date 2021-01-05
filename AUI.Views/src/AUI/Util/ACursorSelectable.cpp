/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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

//
// Created by alex2 on 30.11.2020.
//

#include <AUI/Platform/AInput.h>
#include <AUI/Render/Render.h>
#include <AUI/Render/RenderHints.h>
#include "ACursorSelectable.h"

ACursorSelectable::Selection ACursorSelectable::getSelection() const {
    return { glm::min(mCursorIndex, mCursorSelection), glm::max(mCursorIndex, mCursorSelection) };
}

bool ACursorSelectable::hasSelection() const {
    return mCursorIndex != mCursorSelection && mCursorSelection != -1;
}

unsigned ACursorSelectable::getCursorIndexByPos(glm::ivec2 pos) {
    if (pos.x < 0)
        return 0;

    auto text = getMouseSelectionText();
    if (text.empty()) {
        return 0;
    }

    const auto& f = getMouseSelectionFont();

    pos = pos - (getMouseSelectionPadding() - getMouseSelectionScroll());

    auto fs = getMouseSelectionFont();
    int row = pos.y < 0 ? 0 : pos.y / fs.getLineHeight();

    if (row == 0) {
        return f.font->indexOfX(text, pos.x, f.size, f.fontRendering);
    }

    // oh! we should even find this row...
    size_t targetLineIndex = 0;
    for (size_t r = 0; r < row; ++r) {
        auto temp = text.find('\n', targetLineIndex);
        if (temp == AString::NPOS) {
            // cursor gone out of selecting AView
            break;
        }
        targetLineIndex = temp + 1;
    }

    return targetLineIndex + f.font->indexOfX(text.mid(targetLineIndex, text.find('\n', targetLineIndex)), pos.x, f.size, f.fontRendering);
}

void ACursorSelectable::handleMousePressed(const glm::ivec2& pos, AInput::Key button) {
    if (button == AInput::LButton) {
        mCursorSelection = mCursorIndex = getCursorIndexByPos(pos);
    }
}

void ACursorSelectable::handleMouseMove(const glm::ivec2& pos) {
    if (AInput::isKeyDown(AInput::LButton)) {
        mCursorIndex = getCursorIndexByPos(pos);
        doRedraw();
    }
}

void ACursorSelectable::handleMouseReleased(const glm::ivec2& pos, AInput::Key button) {
    if (button == AInput::LButton)
    {
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

        Render::inst().setFill(Render::FILL_SOLID);
        RenderHints::PushColor c;
        Render::inst().setColor(AColor(1.f) - AColor(0x0078d700u));
        
        auto padding = getMouseSelectionPadding();
        drawSelectionRects();

    }
    return absoluteCursorPos;
}

int ACursorSelectable::getPosByIndex(int end, int begin) {
    return -getMouseSelectionScroll().x + int(getMouseSelectionFont().getWidth(getMouseSelectionText().mid(begin, end - begin)));
}

void ACursorSelectable::drawSelectionPost() {
    Render::inst().setFill(Render::FILL_SOLID);
    Render::inst().setColor({1, 1, 1, 1 });

    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    if (hasSelection())
    {
        drawSelectionRects();
    }
}

void ACursorSelectable::selectAll() {
    mCursorSelection = 0;
    mCursorIndex = getText().length();
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
        Render::inst().drawRect(p.x + absoluteBeginPos,
                                    p.y + row * fs.getLineHeight(),
                                    absoluteEndPos - absoluteBeginPos + 1,
                                    getMouseSelectionFont().size + 2);
    };

    auto t = getMouseSelectionText();
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
    auto text = getMouseSelectionText();

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

