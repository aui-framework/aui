#include "MainWindow.h"
#include <AUI/Util/kAUI.h>

int main()
{
    Stylesheet::instance().load(AUrl(":words/style.less").open());
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