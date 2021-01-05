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

#pragma once


#include <AUI/Common/AString.h>
#include <glm/glm.hpp>
#include <AUI/Render/FontStyle.h>
#include <AUI/Platform/AInput.h>

class API_AUI_VIEWS ACursorSelectable {
private:
    int mAbsoluteBegin, mAbsoluteEnd;

protected:
    unsigned mCursorIndex = 0;
    unsigned mCursorSelection = -1;

    virtual glm::ivec2 getMouseSelectionPadding() = 0;
    virtual glm::ivec2 getMouseSelectionScroll() = 0;
    virtual FontStyle getMouseSelectionFont() = 0;
    virtual AString getMouseSelectionText() = 0;
    virtual void doRedraw() = 0;


    void handleMouseDoubleClicked(const glm::ivec2& pos, AInput::Key key);
    void handleMousePressed(const glm::ivec2& pos, AInput::Key button);
    void handleMouseReleased(const glm::ivec2& pos, AInput::Key button);
    void handleMouseMove(const glm::ivec2& pos);

    /**
     * \return absoluteCursorPos
     */
    int drawSelectionPre();

    void drawSelectionPost();

public:

    struct Selection
    {
        unsigned begin;
        unsigned end;
    };
    [[nodiscard]] virtual const AString& getText() const = 0;
    [[nodiscard]] AString getSelectedText() const
    {
        if (!hasSelection())
            return {};
        return {getText().begin() + getSelection().begin, getText().begin() + getSelection().end};
    }

    [[nodiscard]] Selection getSelection() const;
    [[nodiscard]] bool hasSelection() const;

    [[nodiscard]] unsigned getCursorIndexByPos(glm::ivec2 pos);
    [[nodiscard]] int getPosByIndex(int end, int begin = 0);

    void selectAll();
    void clearSelection();

    void drawSelectionRects();
};


