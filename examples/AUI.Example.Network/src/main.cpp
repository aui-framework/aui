#include <windows.h>
#include "ExampleWindow.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Util/BuiltinFiles.h"

int main()
{
	/*
	{
		BuiltinFiles f;
		ByteBuffer b(AUI_PACKED_asset, sizeof(AUI_PACKED_asset));
		f.loadBuffer(b);
		Stylesheet::inst().load(f.open("assets/views/style.css"));
	}*/
	ExampleWindow w;
	w.loop();
	
	return 0;
}

int __stdcall WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
)
{
	return main();
}