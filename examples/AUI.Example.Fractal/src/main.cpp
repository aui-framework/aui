#include <AUI/Platform/Entry.h>
#include "FractalWindow.h"

AUI_ENTRY
{
    _new<FractalWindow>()->show();
	return 0;
}