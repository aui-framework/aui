// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 30.11.2020.
//

#include <AUI/Platform/AInput.h>
#include <AUI/Render/Render.h>
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

void ACursorSelectable::handleMousePressed(const glm::ivec2& pos, AInput::Key button) {
    if (button == AInput::LBUTTON) {
        mCursorSelection = mCursorIndex = cursorIndexByPos(pos);
    }
}

void ACursorSelectable::handleMouseMove(const glm::ivec2& pos) {
    if (!mIgnoreSelection && isLButtonPressed()) {
        mCursorIndex = cursorIndexByPos(pos);
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
    return -getMouseSelectionScroll().x + int(getMouseSelectionFont().getWidth(getDisplayText().substr(begin, end - begin)));
}

void ACursorSelectable::drawSelectionPost() {
    Render::setBlending(Blending::INVERSE_DST);
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

void ACursorSelectable::handleMouseDoubleClicked(const glm::ivec2& pos, AInput::Key button) {
    mIgnoreSelection = true;
    auto text = getDisplayText();

    // select word
    auto clickIndex = cursorIndexByPos(pos);


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

