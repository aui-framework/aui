/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/api.h>
#include <AUI/Common/AStringVector.h>

/**
 * @def AUI_ENTRY
 * @brief Application entry point.
 * @ingroup useful_macros
 * @details
 * Native entry point varies platform to platform (i.e. Windows requires `int main` entry point for console applications
 * and `WinMain` for graphical applications; entry point of an Android application is located in Java code). AUI_ENTRY
 * unifies your entry point, efficiently supporting every platform.
 *
 * AUI_ENTRY of a graphical application should be non-blocking since on mobile platforms application's event loop is
 * located outsize of the entry point. On desktop platforms an event loop is created outside AUI_ENTRY in order to
 * unify the mobile and desktop behaviour. If there are no open windows, the event loop breaks causing the application
 * to exit.
 *
 * Application arguments (int argc, char** argv) are forwarded to AUI_ENTRY as `AStringVector args`.
 *
 * @note On Android, iOS and macOS only the main thread is able to initialize graphics context.
 */

#if defined(WIN32)
#include <windows.h>

// fake the main function when tests module compiling
#ifdef AUI_TESTS_MODULE
#define AUI_ENTRY \
    AUI_EXPORT int aui_entry(AStringVector args); \
    AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(AStringVector)); \
    int fake_main(int argc, char** argv) {                               \
        return aui_main(argc, argv, aui_entry);\
    }             \
AUI_EXPORT int aui_entry(AStringVector args)
#else
    #define AUI_ENTRY \
    AUI_EXPORT int aui_entry(AStringVector args); \
    AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(AStringVector)); \
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
AUI_EXPORT int aui_entry(AStringVector args)
#endif
#elif AUI_PLATFORM_ANDROID

#include <jni.h>

#define AUI_ENTRY \
    AUI_EXPORT int aui_entry(AStringVector args); \
    AUI_EXPORT int aui_main(JavaVM* vm, int(*aui_entry)(AStringVector)); \
extern "C" \
JNIEXPORT jint JNICALL \
JNI_OnLoad(JavaVM* vm, void* reserved) { \
        aui_main(vm, aui_entry); \
        return JNI_VERSION_1_2;  \
    } \
    AUI_EXPORT int aui_entry(AStringVector args)

#else

// fake the main function when tests module compiling
#ifdef AUI_TESTS_MODULE
#define AUI_ENTRY \
    AUI_EXPORT int aui_entry(AStringVector args); \
    AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(AStringVector)); \
    int fake_main(int argc, char** argv) {                               \
        return aui_main(argc, argv, aui_entry);\
    } \
    AUI_EXPORT int aui_entry(AStringVector args)
#else
#define AUI_ENTRY \
    AUI_EXPORT int aui_entry(AStringVector args); \
    AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(AStringVector)); \
    int main(int argc, char** argv) {                               \
        return aui_main(argc, argv, aui_entry);\
    } \
    AUI_EXPORT int aui_entry(AStringVector args)
#endif

#endif