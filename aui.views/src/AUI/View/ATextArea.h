// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
// Created by alex2 on 5/22/2021.
//


#pragma once


#include "AViewContainer.h"
#include "AAbstractTextField.h"
#include "AScrollbar.h"

/**
 * @brief Multiline text field.
 * @ingroup useful_views
 * @details
 * Word breaking text area.
 */
class API_AUI_VIEWS ATextArea: public AViewContainer {
private:
    class TextAreaField;
    _<TextAreaField> mTextField;
    _<AScrollbar> mScrollbar;
    bool mEditable = false; // TODO editable

public:
    ATextArea();
    explicit ATextArea(const AString& text);

    int getContentMinimumHeight(ALayoutDirection layout) override;

    void onMouseWheel(glm::ivec2 pos, glm::ivec2 delta) override;
};


