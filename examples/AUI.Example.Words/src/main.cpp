#include "MainWindow.h"
#include <AUI/Util/kAUI.h>

int main()
{
    AString a = ".icon { padding: 0; -aui-font-rendering: antialiasing;}";
    Stylesheet::instance().load(a);
	auto w = _new<MainWindow>();
	w->loop();
	return 0;
}
/*
#ifdef _WIN32
#include <Windows.h>
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