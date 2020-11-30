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
    virtual const FontStyle& getMouseSelectionFont() = 0;
    virtual AString getMouseSelectionText() = 0;
    virtual void doRedraw() = 0;

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
        return {getText().begin() + getSelection().begin, getText().begin() + getSelection().end};
    }

    [[nodiscard]] Selection getSelection() const;
    bool hasSelection();

    unsigned getCursorIndexByPos(const glm::ivec2& pos);
    int getPosByIndex(int index);

    void selectAll();
};


