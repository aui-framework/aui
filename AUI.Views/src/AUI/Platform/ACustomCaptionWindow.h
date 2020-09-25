#pragma once
#include "ACustomWindow.h"

class API_AUI_VIEWS ACustomCaptionWindow: public ACustomWindow
{
private:
	_<AViewContainer> mCaptionContent;
	
public:
	
	ACustomCaptionWindow(const AString& name, int width, int height);

	ACustomCaptionWindow():
            ACustomCaptionWindow("Custom Caption Window", 240, 124)
	{
	}

	virtual ~ACustomCaptionWindow() = default;


	[[nodiscard]] const _<AViewContainer>& getCaptionContent() const
	{
		return mCaptionContent;
	}
};
