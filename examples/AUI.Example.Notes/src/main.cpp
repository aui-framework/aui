#include "MainWindow.h"
#include <AUI/Util/kAUI.h>
#include <AUI/Platform/Entry.h>

AUI_ENTRY
{
    Stylesheet::setPreferredStyle(Stylesheet::PREFER_UNIVERSAL_LOOK);
    Stylesheet::inst().load(AUrl(":words/style.less").open());
	_new<MainWindow>()->show();
	return 0;
}