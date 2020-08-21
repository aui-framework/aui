#include "ExampleWindow.h"
#include "AUI/Common/ByteBuffer.h"
#include "AUI/Util/BuiltinFiles.h"
#include "assets.h"

int main()
{
	{
		BuiltinFiles f;
		ByteBuffer b(AUI_PACKED_asset, sizeof(AUI_PACKED_asset));
		f.loadBuffer(b);
		Stylesheet::instance().load(f.open("assets/views/style.css"));
	}
	ExampleWindow w;
	w.loop();
	return 0;
}

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