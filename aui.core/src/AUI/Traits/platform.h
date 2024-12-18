/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <type_traits>

namespace aui::platform {
    namespace mobile {
        using is_vertical_layout_preferred = std::true_type;

        constexpr bool is_mobile() {
            return true;
        }
    }

    namespace desktop {
        using is_vertical_layout_preferred = std::false_type;

        constexpr bool is_mobile() {
            return false;
        }
    }

    namespace os_windows {
        constexpr char path_variable_separator = ';';
        using namespace desktop;

#if AUI_PLATFORM_WIN
#endif

        constexpr bool is_windows() {
            return true;
        }

        constexpr bool is_unix() {
            return false;
        }
        constexpr bool is_apple() {
            return false;
        }

        static const char* name() {
            return "windows";
        }

    }
    namespace os_unix {
        using namespace desktop;

        constexpr char path_variable_separator = ':';

        static const char* name() {
            return "unix";
        }

        constexpr bool is_windows() {
            return false;
        }

        constexpr bool is_unix() {
            return true;
        }
        constexpr bool is_apple() {
            return false;
        }
    }
    namespace macos {
        using namespace desktop;

        constexpr char path_variable_separator = ':';

        static const char* name() {
            return "osx";
        }

        constexpr bool is_windows() {
            return false;
        }

        constexpr bool is_unix() {
            return true;
        }
        constexpr bool is_apple() {
            return true;
        }
    }
    namespace os_android {
        using namespace mobile;

        constexpr char path_variable_separator = ':';

        static const char* name() {
            return "android";
        }


        constexpr bool is_windows() {
            return false;
        }

        constexpr bool is_unix() {
            return true;
        }

        constexpr bool is_apple() {
            return false;
        }
    }
    namespace os_ios {
        using namespace mobile;

        constexpr char path_variable_separator = ':';

        static const char* name() {
            return "ios";
        }


        constexpr bool is_windows() {
            return false;
        }

        constexpr bool is_unix() {
            return true;
        }

        constexpr bool is_apple() {
            return true;
        }
    }

    constexpr bool is_64_bit = sizeof(void*) == 8;
    constexpr bool is_32_bit = sizeof(void*) == 4;

    constexpr bool is_x86 = AUI_ARCH_X86;
    constexpr bool is_x86_64 = AUI_ARCH_X86_64;

    /**
     * @brief ARM 32 bit.
     */
    constexpr bool is_armv7 = AUI_ARCH_ARM_V7;

    /**
     * @brief ARM 64 bit architecture, also known as aarch64.
     */
    constexpr bool is_arm64 = AUI_ARCH_ARM_64;

#if AUI_PLATFORM_WIN
    namespace current = os_windows;
#elif AUI_PLATFORM_ANDROID
    namespace current = os_android;
#elif AUI_PLATFORM_MACOS
    namespace current = macos;
#elif AUI_PLATFORM_IOS
    namespace current = os_ios;
#else
    namespace current = os_unix;
#endif
}
