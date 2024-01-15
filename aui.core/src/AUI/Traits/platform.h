// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
