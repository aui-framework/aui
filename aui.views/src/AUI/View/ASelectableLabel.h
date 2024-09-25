/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

    void render(ARenderContext context) override;

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