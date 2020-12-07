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

bool ACursorSelectable::hasSelection() {
    return mCursorIndex != mCursorSelection && mCursorSelection != -1;
}

unsigned ACursorSelectable::getCursorIndexByPos(glm::ivec2 pos) {
    if (pos.y < 0)
        return 0;
    auto text = getMouseSelectionText();
    if (text.empty()) {
        return 0;
    }

    const auto& f = getMouseSelectionFont();

    pos = pos - (getMouseSelectionPadding() - getMouseSelectionScroll());

    auto fs = getMouseSelectionFont();
    int row = pos.y / fs.getLineHeight();
    if (row < 0) {
        row = 0;
    }

    if (row == 0) {
        return f.font->indexOfX(text, pos.x, f.size, f.fontRendering);
    }

    // твою мышь! теперь ещё надо найти эту строчку...
    size_t targetLineIndex = 0;
    for (size_t r = 0; r < row; ++r) {
        targetLineIndex = text.find('\n', targetLineIndex);
        if (targetLineIndex == AString::NPOS) {
            // курсор вышел за границу выделяемого AView. нетрудно догадаться, что в этом случае мы можем схалявить
            // и вернуть просто индекс последнего элемента
            return text.length() - 1;
        }
        targetLineIndex += 1;
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
    
    // выделение
    if (hasSelection())
    {
        auto absoluteSelectionPos = getPosByIndex(mCursorSelection);

        mAbsoluteBegin = glm::min(absoluteCursorPos, absoluteSelectionPos);
        mAbsoluteEnd = glm::max(absoluteCursorPos, absoluteSelectionPos);

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

