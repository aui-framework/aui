#include <AUI/Platform/Entry.h>
#include "MinesweeperWindow.h"

AUI_ENTRY
{
	_new<MinesweeperWindow>()->show();

	return 0;
}