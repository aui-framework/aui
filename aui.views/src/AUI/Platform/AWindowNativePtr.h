//
// Created by Alex2772 on 12/7/2021.
//

#pragma once

#if AUI_PLATFORM_WIN
#include <Windows.h>
using AWindowNativePtr = HWND;

#elif AUI_PLATFORM_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

using AWindowNativePtr = Window;
#endif