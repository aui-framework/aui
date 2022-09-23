#pragma once

#include "AUI/Image/AImage.h"
#include <Windows.h>

namespace aui::win32 {
    AImage iconToImage(HICON hIcon);
    AImage bitmapToImage(HBITMAP hbitmap);
}


