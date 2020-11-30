//
// Created by alex2 on 30.11.2020.
//

#pragma once


#include <AUI/Util/ACursorSelectable.h>
#include "ALabel.h"

class API_AUI_VIEWS ASelectableLabel: public ALabel, public ACursorSelectable {
public:
    ASelectableLabel();
    explicit ASelectableLabel(const AString& text);

    [[nodiscard]] const AString& getText() const override
    {
        return ALabel::getText();
    }

    void render() override;

    void onMouseMove(glm::ivec2 pos) override;
    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;
    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

    void onFocusLost() override;

    void onKeyDown(AInput::Key key) override;

protected:
    glm::ivec2 getMouseSelectionPadding() override;
    glm::ivec2 getMouseSelectionScroll() override;
    FontStyle getMouseSelectionFont() override;
    AString getMouseSelectionText() override;
    void doRedraw() override;

};