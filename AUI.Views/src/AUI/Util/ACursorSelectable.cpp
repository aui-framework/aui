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

unsigned ACursorSelectable::getCursorIndexByPos(const glm::ivec2& pos) {
    const auto& f = getMouseSelectionFont();

    auto posX = pos.x;
    posX = posX - (getMouseSelectionPadding().x + getMouseSelectionScroll().x);
    if (posX <= 0)
        return 0;
    return f.font->trimStringToWidth(getMouseSelectionText(), posX, f.size, f.fontRendering).length();
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

        Render::instance().setFill(Render::FILL_SOLID);
        RenderHints::PushColor c;
        Render::instance().setColor(AColor(1.f) - AColor(0x0078d700u));
        
        auto padding = getMouseSelectionPadding();
        Render::instance().drawRect(padding.x + mAbsoluteBegin, padding.y, mAbsoluteEnd - mAbsoluteBegin + 1, getMouseSelectionFont().size + 3);

        
        Render::instance().drawRect(padding.x + mAbsoluteBegin, padding.y, mAbsoluteEnd - mAbsoluteBegin + 1,
                                    getMouseSelectionFont().size + 3);
    }
    return absoluteCursorPos;
}

int ACursorSelectable::getPosByIndex(int index) {
    return -getMouseSelectionScroll().x + int(getMouseSelectionFont().getWidth(getMouseSelectionText().mid(0, index)));
}

void ACursorSelectable::drawSelectionPost() {
    Render::instance().setFill(Render::FILL_SOLID);
    Render::instance().setColor({ 1, 1, 1, 1 });

    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    if (hasSelection())
    {
        auto p = getMouseSelectionPadding();
        Render::instance().drawRect(p.x + mAbsoluteBegin,
                                    p.y,
                                    mAbsoluteEnd - mAbsoluteBegin + 1,
                                    getMouseSelectionFont().size + 3);
    }
}

void ACursorSelectable::selectAll() {
    mCursorSelection = 0;
    mCursorIndex = getText().length();
}

