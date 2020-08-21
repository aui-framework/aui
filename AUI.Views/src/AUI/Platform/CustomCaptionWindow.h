#pragma once
#include "CustomWindow.h"

class API_AUI_VIEWS CustomCaptionWindow: public CustomWindow
{
private:
	_<AViewContainer> mCaptionContent;
	
public:
	
	CustomCaptionWindow(const AString& name, int width, int height);

	CustomCaptionWindow():
		CustomCaptionWindow("Custom Caption Window", 240, 124)
	{
	}

	virtual ~CustomCaptionWindow() = default;


	[[nodiscard]] const _<AViewContainer>& getCaptionContent() const
	{
		return mCaptionContent;
	}
};
