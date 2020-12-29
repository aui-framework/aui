#include "ExampleWindow.h"
#include <AUI/Platform/Entry.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/ASS/AStylesheet.h>

AUI_ENTRY
{
    //Stylesheet::inst().load(AUrl(":views/style.css").open());
	auto w = _new<ExampleWindow>();
	w->show();
	return 0;
}