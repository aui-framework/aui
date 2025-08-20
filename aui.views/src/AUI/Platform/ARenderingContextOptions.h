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

#include <AUI/Common/AVector.h>
#include <variant>
#include "AUI/Reflect/AEnumerate.h"



AUI_ENUM_FLAG(ARenderContextFlags) {
    DEFAULT = 0b0,

    /**
     * @brief Controls AUI's smooth window resize feature. If set to false, window's would resize dirty, however,
     * performance is increased by 10-50% especially in games.
     */
    NO_SMOOTH = 0b1,

    /**
     * @brief Disables vsync, causing the machine render as frequently as possible.
     */
    NO_VSYNC = 0b10,
};

/**
 * @brief Defines rendering API priority and options for your application.
 * @ingroup views
 * @details
 * By default, AUI uses the following settings:
 * ```cpp
 * ARenderingContextOptions::set({
 *    OpenGL {},
 *    Software {},
 * });
 * ```
 * You may call this before initializating windows in order to define your own settings and priorities.
 *
 * The priority can be overridden by `--aui-renderer=GRAPHICS_API` argument passed to your application.
 */
struct API_AUI_VIEWS ARenderingContextOptions {
    struct DirectX11 {
        int majorVersion;
    };

    struct OpenGL {
        int majorVersion = 2, minorVersion = 0;

        enum class Profile {
            CORE,
            COMPAT
        } profile = Profile::CORE;
    };

    struct Software {};

    using InitializationVariant =  std::variant<DirectX11,
            OpenGL,
            Software>;

    AVector<InitializationVariant> initializationOrder;

    ARenderContextFlags flags = ARenderContextFlags::DEFAULT;

    static void set(ARenderingContextOptions options) {
        inst() = std::move(options);
    }
    static const ARenderingContextOptions& get() noexcept {
        return inst();
    }

private:
    static ARenderingContextOptions& inst();
};

AUI_ENUM_VALUES(ARenderingContextOptions::OpenGL::Profile,
                ARenderingContextOptions::OpenGL::Profile::CORE,
                ARenderingContextOptions::OpenGL::Profile::COMPAT)
