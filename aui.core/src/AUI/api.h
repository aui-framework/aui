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

#ifdef AUI_STATIC
    #define AUI_IMPORT
    #define AUI_EXPORT
#else
    #if AUI_PLATFORM_WIN
        #define AUI_IMPORT __declspec(dllimport)
        #define AUI_EXPORT __declspec(dllexport)
    #else
        #define AUI_IMPORT
        #define AUI_EXPORT __attribute__((visibility("default")))
    #endif
#endif


#if !__cpp_rtti
#error "You have RTTI disabled."
#endif
