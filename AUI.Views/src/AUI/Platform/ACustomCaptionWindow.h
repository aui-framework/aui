#pragma once
#include "ACustomWindow.h"
#include <AUI/View/AButton.h>
#include "CustomCaptionWindowImplWin32.h"

using CustomCaptionWindowImplCurrent = CustomCaptionWindowImplWin32;

class API_AUI_VIEWS ACustomCaptionWindow: public ACustomWindow, public CustomCaptionWindowImplCurrent
{

protected:
    bool isCustomCaptionMaximized() override;

public:

	ACustomCaptionWindow(const AString& name, int width, int height, bool stacked = false);

	ACustomCaptionWindow():
            ACustomCaptionWindow("Custom Caption Window", 240, 124)
	{
	}



	virtual ~ACustomCaptionWindow() = default;
};
