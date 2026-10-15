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

#if defined(__APPLE__) || AUI_PLATFORM_APPLE

#include <CoreFoundation/CoreFoundation.h>
#include <memory>
#include <type_traits>

/**
 * @brief Custom deleter for CoreFoundation objects that calls CFRelease.
 * @ingroup core
 * @exclusivefor{apple}
 */
struct CFReleaseDeleter {
    void operator()(CFTypeRef cf) const noexcept {
        if (cf) {
            CFRelease(cf);
        }
    }
};

/**
 * @brief std::unique_ptr alias for CoreFoundation reference types (CFStringRef, CTFontRef, CFURLRef, etc.)
 * @ingroup core
 * @tparam T CoreFoundation reference type
 * @details Automatically calls CFRelease upon destruction.
 * @exclusivefor{apple}
 */
template<typename T>
using ACFPtr = std::unique_ptr<std::remove_pointer_t<T>, CFReleaseDeleter>;

#endif
