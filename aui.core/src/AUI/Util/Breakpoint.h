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

#if !AUI_PLATFORM_WIN
#include <signal.h>
#endif

/**
 * @brief Stops program execution in attached debugger as if a breakpoint were set.
 * @ingroup useful_macros
 * @details
 * If debugger is not present, behaviour is undefined.
 *
 * ```cpp
 * AUI_BREAKPOINT(); // stops debugger here.
 * ```
 */
#define AUI_BREAKPOINT() AUI_IMPL_BREAKPOINT

#if AUI_PLATFORM_WIN
#define AUI_IMPL_BREAKPOINT __debugbreak()
#else
#define AUI_IMPL_BREAKPOINT raise(SIGTRAP)
#endif
