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
// Created by alex2 on 05.12.2020.
//

#pragma once

#include <AUI/View/AViewContainer.h>
#include <AUI/View/AButton.h>

class API_AUI_VIEWS CustomCaptionWindowImplWin32 {
protected:
    _<AViewContainer> mCaptionContainer;
    _<AViewContainer> mContentContainer;
    _<AButton> mMinimizeButton; // _
    _<AButton> mMiddleButton; // []
    _<AButton> mCloseButton; // X

    void updateMiddleButtonIcon();
    void initCustomCaption(const AString& name, bool stacked, AViewContainer* to);

    virtual bool isCustomCaptionMaximized() = 0;

public:
    CustomCaptionWindowImplWin32() = default;

    virtual ~CustomCaptionWindowImplWin32() = default;

    [[nodiscard]] const _<AViewContainer>& getCaptionContainer() const
    {
        return mCaptionContainer;
    }

    [[nodiscard]] const _<AViewContainer>& getContentContainer() const
    {
        return mContentContainer;
    }
};


