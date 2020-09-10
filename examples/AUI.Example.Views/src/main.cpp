#include "ExampleWindow.h"
#include "AUI/Common/ByteBuffer.h"
#include "AUI/Util/BuiltinFiles.h"

int main()
{
    Stylesheet::setPreferredStyle(Stylesheet::PREFER_UNIVERSAL_LOOK);
    Stylesheet::instance().load(AUrl(":views/style.css").open());
	ExampleWindow w;
	w.loop();
	return 0;
}

#if defined(_WIN32)
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