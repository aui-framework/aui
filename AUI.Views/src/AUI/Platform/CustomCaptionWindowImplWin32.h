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


