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
#pragma warning(disable: 4251)

#include "AUI/api.h"
#include "AUI/Util/Assert.h"

#if AUI_ENABLE_DEATH_TESTS
#define AUI_EXPECT_DEATH(statement, matcher) EXPECT_DEATH(statement, matcher)
#else
#define AUI_EXPECT_DEATH(statement, matcher)
#endif


/**
 * @defgroup core aui::core
 * @brief Main central library.
 * @details AUI's core central library without graphical components used by other modules.
 * Adds these features to C++:
 * - @ref signal_slot "signal-slot system"
 * - @ref property_system "property system"
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
 * @brief A set of functionality that introduces non-standard syntax which seems like the language features.
 * @details
 * @note
 * For build-time macros, see @ref "docs/Macros.md".
 */

/**
 * @defgroup io IO streams
 * @ingroup core
 * @brief Input-output streams
 */

