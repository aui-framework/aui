//
// Created by alex2 on 05.12.2020.
//

#include "CustomCaptionWindowImplWin32.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Common/Plugin.h>

void CustomCaptionWindowImplWin32::initCustomCaption(const AString& name, bool stacked, AViewContainer* to) {
    auto caption = _new<AViewContainer>();
    caption->setLayout(_new<AHorizontalLayout>());
    caption->addCssName(".window-title");
    caption->setExpanding({1, 0});

    auto titleLabel = _new<ALabel>(name);
    caption->addView(titleLabel);

    mCaptionContainer = _new<AViewContainer>();
    mCaptionContainer->setLayout(_new<AHorizontalLayout>());
    mCaptionContainer->setExpanding({1, 0 });
    mCaptionContainer->addCssName(".window-title-content");
    caption->addView(mCaptionContainer);

    mMinimizeButton = _new<AButton>();
    mMinimizeButton->addCssName(".minimize");
    mMinimizeButton->addCssName(".default");
    caption->addView(mMinimizeButton);

    mMiddleButton = _new<AButton>();
    mMiddleButton->addCssName(".middle");
    mMiddleButton->addCssName(".default");

    caption->addView(mMiddleButton);

    mCloseButton = _new<AButton>();
    mCloseButton->addCssName(".close");
    mCloseButton->addCssName(".default");
    caption->addView(mCloseButton);

    if (stacked) {
        to->setLayout(_new<AStackedLayout>());
        to->addView(mContentContainer = _new<AViewContainer>());
        to->addView(_container<AVerticalLayout>({
                                                    caption,
                                                    _new<ASpacer>(),
                                            }) let (AViewContainer, {
                setExpanding({1, 1});
        }));
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
        mMiddleButton->setCss("background: url(':win/caption/restore.svg')");
    } else {
        mMiddleButton->setCss("background: url(':win/caption/maximize.svg')");
    }
}
