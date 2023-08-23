// AUI Framework - Declarative UI toolkit for modern C++20
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

#pragma once


#include <AUI/Util/ACursorSelectable.h>
#include "ALabel.h"

/**
 * @brief Like ALabel but with cursor selection capabilities.
 */
class API_AUI_VIEWS ASelectableLabel: public ALabel, public ACursorSelectable {
public:
    ASelectableLabel();
    explicit ASelectableLabel(const AString& text);

    [[nodiscard]] const AString& text() const override
    {
        return ALabel::text();
    }

    size_t textLength() const override;

    void render() override;

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;
    void onPointerPressed(const APointerPressedEvent& event) override;
    void onPointerReleased(const APointerReleasedEvent& event) override;

    void onFocusLost() override;

    bool consumesClick(const glm::ivec2& pos) override;

    void onFocusAcquired() override;

    void onKeyDown(AInput::Key key) override;

    void onPointerDoubleClicked(const APointerPressedEvent& event) override;

protected:
    glm::ivec2 getMouseSelectionPadding() override;
    glm::ivec2 getMouseSelectionScroll() override;
    AFontStyle getMouseSelectionFont() override;
    AString getDisplayText() override;
    void doRedraw() override;

};