#include "CellView.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Platform/ACustomCaptionWindow.h"
#include "AUI/Util/BuiltinFiles.h"
#include "AUI/View/AButton.h"
#include "assets.h"
#include "MinesweeperWindow.h"
#include "AUI/Layout/AStackedLayout.h"

int main()
{
	{
		BuiltinFiles f;
		ByteBuffer b(AUI_PACKED_asset, sizeof(AUI_PACKED_asset));
		f.loadBuffer(b);
		Stylesheet::inst().load(f.open("assets/minesweeper/style.css"));
	}
	
	MinesweeperWindow w;
	
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