#include "MainWindow.h"
#include <AUI/Util/kAUI.h>
#include <AUI/Platform/Entry.h>

AUI_ENTRY
{
    Stylesheet::setPreferredStyle(Stylesheet::PREFER_UNIVERSAL_LOOK);
    Stylesheet::instance().load(AUrl(":words/style.less").open());
	auto w = _new<MainWindow>();
	w->loop();
	return 0;
}