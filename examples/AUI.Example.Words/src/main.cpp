#include "MainWindow.h"
#include <AUI/Util/kAUI.h>
#include <AUI/Platform/Entry.h>

AUI_ENTRY
{
    Stylesheet::setPreferredStyle(Stylesheet::PREFER_UNIVERSAL_LOOK);
    Stylesheet::instance().load(AUrl(":words/style.less").open());
	auto w = _new<MainWindow>();
	w->show();
	return 0;
}
/*
#if defined(_WIN32)
#include <windows.h>
int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
)
{
	return main();
}
#endif
*/