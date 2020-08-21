#pragma once

#include <AUI/Platform/AWindow.h>

#include "AUI/IO/InputStreamAsync.h"

class ExampleWindow: public AWindow {
private:
	_<InputStreamAsync> mInput;
	
public:
	ExampleWindow();
};
