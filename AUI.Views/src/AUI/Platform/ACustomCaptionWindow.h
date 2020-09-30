#pragma once
#include "ACustomWindow.h"
#include <AUI/View/AButton.h>

class API_AUI_VIEWS ACustomCaptionWindow: public ACustomWindow
{
private:
    _<AViewContainer> mCaptionContainer;
    _<AViewContainer> mContentContainer;
    _<AButton> mMiddle;

    void updateMiddleButtonIcon();

public:

	ACustomCaptionWindow(const AString& name, int width, int height, bool stacked = false);

	ACustomCaptionWindow():
            ACustomCaptionWindow("Custom Caption Window", 240, 124)
	{
	}


	virtual ~ACustomCaptionWindow() = default;
    [[nodiscard]] const _<AViewContainer>& getCaptionContainer() const
	{
		return mCaptionContainer;
	}

	[[nodiscard]] const _<AViewContainer>& getContentContainer() const
	{
		return mContentContainer;
	}
};
