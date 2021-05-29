/**
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <type_traits>

namespace aui::platform {
    namespace mobile {
        using is_vertical_layout_preferred = std::true_type;
    }

    namespace desktop {
        using is_vertical_layout_preferred = std::false_type;
    }

    namespace os_windows {
        constexpr char path_variable_separator = ';';
        using namespace desktop;

#ifdef _WIN32
#endif

        constexpr bool is_windows() {
            return true;
        }

        constexpr bool is_unix() {
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
    }

#if defined(_WIN32)
    namespace current = os_windows;
#elif defined(__ANDROID__)
    namespace current = os_android;
#else
    namespace current = os_unix;
#endif
}
