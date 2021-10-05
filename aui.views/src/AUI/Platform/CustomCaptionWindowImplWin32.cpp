/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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

    auto titleLabel = _new<ALabel>(name);
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
                                                    _new<ASpacer>(),
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
        mMiddleButton->setCustomAss({
            ass::BackgroundImage {":uni/caption/restore.svg" }
        });
    } else {
        mMiddleButton->setCustomAss({
            ass::BackgroundImage {":uni/caption/maximize.svg" }
        });
    }
}
