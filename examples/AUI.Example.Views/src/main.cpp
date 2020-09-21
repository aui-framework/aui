#include "ExampleWindow.h"
#include <AUI/Platform/Entry.h>

AUI_ENTRY
{
    //Stylesheet::setPreferredStyle(Stylesheet::PREFER_UNIVERSAL_LOOK);
    Stylesheet::instance().load(AUrl(":views/style.css").open());
	auto w = _new<ExampleWindow>();
	w->show();
	return 0;
}