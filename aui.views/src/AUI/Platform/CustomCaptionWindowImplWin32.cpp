// AUI Framework - Declarative UI toolkit for modern C++17
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
// Created by alex2 on 05.12.2020.
//

#include "CustomCaptionWindowImplWin32.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Common/Plugin.h>
#include <AUI/ASS/Declaration/BackgroundImage.h>

void CustomCaptionWindowImplWin32::initCustomCaption(const AString& name, bool stacked, AViewContainer* to) {
    auto caption = _new<AViewContainer>();
    caption->setLayout(_new<AHorizontalLayout>());
    caption->addAssName(".window-title");
    caption->setExpanding({1, 0});

    auto titleLabel = _new<ALabel>(name) << ".title";
    caption->addView(titleLabel);

    mCaptionContainer = _new<AViewContainer>();
    mCaptionContainer->setLayout(_new<AHorizontalLayout>());
    mCaptionContainer->setExpanding({1, 0 });
    mCaptionContainer->addAssName(".window-title-content");
    caption->addView(mCaptionContainer);

    mMinimizeButton = _new<AButton>();
    mMinimizeButton->addAssName(".minimize");
    mMinimizeButton->addAssName(".default");
    caption->addView(mMinimizeButton);

    mMiddleButton = _new<AButton>();
    mMiddleButton->addAssName(".middle");
    mMiddleButton->addAssName(".default");

    caption->addView(mMiddleButton);

    mCloseButton = _new<AButton>();
    mCloseButton->addAssName(".close");
    mCloseButton->addAssName(".default");
    caption->addView(mCloseButton);

    if (stacked) {
        to->setLayout(_new<AStackedLayout>());
        to->addView(mContentContainer = _new<AViewContainer>());
        to->addView(_container<AVerticalLayout>({
                                                    caption,
                                                    _new<ASpacerExpanding>(),
                                            }) let { it->setExpanding({1, 1}); });
    } else {
        to->setLayout(_new<AVerticalLayout>());
        to->addView(caption);
        to->addView(mContentContainer = _new<AViewContainer>());
    }
    mContentContainer->setExpanding({1, 1});

    updateMiddleButtonIcon();
}

void CustomCaptionWindowImplWin32::updateMiddleButtonIcon() {
    if (isCustomCaptionMaximized()) {
        mMiddleButton->setCustomStyle({
            ass::BackgroundImage {":uni/caption/restore.svg" }
        });
    } else {
        mMiddleButton->setCustomStyle({
            ass::BackgroundImage {":uni/caption/maximize.svg" }
        });
    }
}
