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


/**
 * @defgroup core aui::core
 * @brief Main central library.
 * @details AUI's core central library without graphical components used by other modules.
 * Adds these features to C++:
 * - @ref signal_slot "signal-slot system"
 * - @ref AAbstractThread::interrupt() "interruptable threads"
 * - @ref io "input-output"
 * - @ref AI18n "i18n"
 * - @ref ALogger "logging"
 * - data models
 * - reflection
 * - trait classes
 * - @ref AUrl "urls"
 */


/**
 * @defgroup useful_macros Useful macros
 * @ingroup core
 * @brief A set of functionality that introduces non-standard syntax which seems like the C++ features.
 */

/**
 * @defgroup io IO streams
 * @ingroup core
 * @brief Input-output streams
 */


/**
 * @mainpage AUI Framework
 * @htmlinclude ./docs/index.html
 */

#if AUI_PLATFORM_WIN
#include <windows.h>

#endif