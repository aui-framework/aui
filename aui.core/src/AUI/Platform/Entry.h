/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/api.h>
#include <AUI/Util/ACommandLineArgs.h>

/**
 * @def AUI_ENTRY
 * @brief Application entry point.
 * @ingroup useful_macros
 * @details
 * Entry point of any AUI application.
 *
 * Native entry point varies platform to platform (i.e. Windows requires `int main` entry point for console applications
 * and `WinMain` for graphical applications; entry point of an Android application is located in Java code). `AUI_ENTRY`
 * unifies your entry point, effectively supporting every platform.
 *
 * `AUI_ENTRY` of a graphical application should be non-blocking since on mobile platforms application's event loop is
 * located outsize of the entry point. On desktop platforms, an event loop is created outside `AUI_ENTRY` in order to
 * unify the mobile and desktop behaviour. If there are no open windows, the event loop breaks, causing the application
 * to exit with the exit code returned by `AUI_ENTRY` earlier, or `0`.
 *
 * Application arguments `(int argc, char** argv)` are forwarded to `AUI_ENTRY` as `AStringVector args`.
 *
 * Minimal entrypoint of an UI application:
 *
 * <!-- aui:include examples/ui/views/src/main.cpp -->
 *
 * Minimal entrypoint of a console application:
 * <!-- aui:include examples/basic/hello_world/src/main.cpp -->
 *
 * Prototype of `AUI_ENTRY` function you are actually implementing:
 * ```cpp
 * AUI_EXPORT int aui_entry(const AStringVector& args);
 * ```
 *
 * @specificto{android}
 * Only the main thread is able to initialize graphics context.
 *
 * @specificto{ios}
 * Only the main thread is able to initialize graphics context.
 *
 * @specificto{macos}
 * Only the main thread is able to initialize graphics context.
 */

#if defined(WIN32)
#include <windows.h>

// fake the main function when tests module compiling
#ifdef AUI_TESTS_MODULE
#define AUI_ENTRY \
    AUI_EXPORT int aui_entry(const AStringVector& args); \
    AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)); \
    static int fake_main(int argc, char** argv) {                               \
        return aui_main(argc, argv, aui_entry);\
    }             \
AUI_EXPORT int aui_entry(const AStringVector& args)
#else
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
#endif
#elif AUI_PLATFORM_ANDROID

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

// fake the main function when tests module compiling
#ifdef AUI_TESTS_MODULE
#define AUI_ENTRY \
    AUI_EXPORT static int aui_entry(const AStringVector& args); \
    AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)); \
    static int fake_main(int argc, char** argv) {                               \
        return aui_main(argc, argv, aui_entry);\
    } \
    AUI_EXPORT static int aui_entry(const AStringVector& args)
#else
#define AUI_ENTRY \
    AUI_EXPORT int aui_entry(const AStringVector& args); \
    AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)); \
    int main(int argc, char** argv) {                               \
        return aui_main(argc, argv, aui_entry);\
    } \
    AUI_EXPORT int aui_entry(const AStringVector& args)
#endif

#endif

namespace aui {
    /**
     * @ingroup core
     * @return Arguments passed to program.
     */
    API_AUI_CORE const ACommandLineArgs& args() noexcept;
}