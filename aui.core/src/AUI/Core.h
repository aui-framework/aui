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
#pragma warning(disable: 4251)

#include <AUI/api.h>
#include <AUI/Util/Assert.h>

#if AUI_ENABLE_DEATH_TESTS
#define AUI_EXPECT_DEATH(statement, matcher) EXPECT_DEATH(statement, matcher)
#else
#define AUI_EXPECT_DEATH(statement, matcher)
#endif


/**
 * @mainpage AUI Framework
 * @htmlinclude ./docs/index.html
 */


/**
 * @defgroup core aui::core
 * @brief Main central library.
 * @details
 * `aui::core` is the main module of AUI. All other modules are built on top of `aui::core`. It implements several basic
 * features:
 *
 * - [interruptable threads](AAbstractThread::interrupt())
 * - [i18n](ai18n.md)
 * - [logging](alogger.md)
 * - data models
 * - trait classes
 * - [urls](aurl.md)
 */



/**
 * @defgroup useful_macros Useful macros
 * @ingroup core
 * @brief A set of functionality that introduces non-standard syntax which seems like the language features.
 * @details
 */

/**
 * @defgroup io IO streams
 * @ingroup core
 * @brief Input-output streams
 */

/**
 * @defgroup reflection Reflection
 * @ingroup core
 * @brief Basic reflection capabilities.
 * @details
 * <!-- aui:experimental -->
 * This component provides functionality for reflection and introspection of classes and their members in compile-time.
 * Some runtime methods are also available.
 */
