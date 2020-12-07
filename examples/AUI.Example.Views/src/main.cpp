#include "ExampleWindow.h"
#include <AUI/Platform/Entry.h>
#include <AUI/i18n/AI18n.h>

AUI_ENTRY
{
    AI18n::instance();
    Stylesheet::setPreferredStyle(Stylesheet::PREFER_UNIVERSAL_LOOK);
    Stylesheet::inst().load(AUrl(":views/style.css").open());
	auto w = _new<ExampleWindow>();
	w->show();
	return 0;
}