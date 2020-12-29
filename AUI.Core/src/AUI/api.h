#pragma once

#ifdef AUI_STATIC
    #define AUI_IMPORT
    #define AUI_EXPORT
#else
    #if defined(_WIN32)
    #define AUI_IMPORT __declspec(dllimport)
    #define AUI_EXPORT __declspec(dllexport)
    #else
    #define AUI_IMPORT
    #define AUI_EXPORT __attribute__((visibility("default")))
    #endif
#endif