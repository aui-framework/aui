//
// Created by alex2 on 30.11.2020.
//

#pragma once


#include <AUI/Common/AString.h>
#include <glm/glm.hpp>
#include <AUI/Render/FontStyle.h>
#include <AUI/Platform/AInput.h>

class ACursorSelectable {
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


