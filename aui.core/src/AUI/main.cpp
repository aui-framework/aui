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
 * - @ref jni "C++/Java JNI bridge"
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