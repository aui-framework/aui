#pragma once

#include <AUI/Platform/AWindow.h>

class ExampleWindow: public AWindow {
private:
	ADeque<_<AWindow>> mWindows;
	
public:
	ExampleWindow();
};