#pragma once

#include <AUI/api.h>
#include <AUI/Common/AStringVector.h>

#if defined(WIN32)
#include <windows.h>
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
#elif defined(__ANDROID__)

#include <jni.h>

#define AUI_ENTRY \
    AUI_EXPORT int aui_entry(const AStringVector& args); \
    AUI_EXPORT int aui_main(JavaVM* vm, int(*aui_entry)(const AStringVector&)); \
extern "C" \
JNIEXPORT jint JNICALL \
JNI_OnLoad(JavaVM* vm, void* reserved) { \
        aui_main(vm, aui_entry); \
        return JNI_VERSION_1_2;  \
    } \
    AUI_EXPORT int aui_entry(const AStringVector& args)

#else
#define AUI_ENTRY \
    AUI_EXPORT int aui_entry(const AStringVector& args); \
    AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)); \
    int main(int argc, char** argv) {                               \
        return aui_main(argc, argv, aui_entry);\
    } \
    AUI_EXPORT int aui_entry(const AStringVector& args)
#endif