#pragma once

#include <AUI/api.h>
#include <AUI/Common/AStringVector.h>

#if defined(WIN32)
#include <Windows.h>
    #define AUI_ENTRY \
    AUI_EXPORT int aui_entry(const AStringVector& args); \
    AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)); \
    int main(int argc, char** argv) {                               \
        return aui_main(argc, argv, aui_entry);\
    } \
    int __stdcall WinMain( \
        HINSTANCE hInstance, \
        HINSTANCE hPrevInstance, \
        LPSTR     lpCmdLine, \
        int       nShowCmd \
) { \
    return main(0, nullptr); \
} \
AUI_EXPORT int aui_entry(const AStringVector& args)
#else
    #define AUI_ENTRY AUI_EXPORT extern "C" int aui_entry(const AStringVector& args)
#endif