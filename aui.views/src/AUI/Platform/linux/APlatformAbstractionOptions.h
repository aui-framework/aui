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

#include "IPlatformAbstraction.h"

/**
 * @brief Defines core platform API priority and options for your application.
 * @ingroup views
 * @details
 * @exclusivefor{linux}
 *
 * This API is similar to ARenderingContextOptions, but defines which core platform API to use.
 */
struct API_AUI_VIEWS APlatformAbstractionOptions {
    /**
     * @brief Corresponds to X11 (legacy) mode.
     * @details
     * While being deprecated, this is still a useful fallback option. If user uses Wayland (which is likely on a modern
     * [linux.md] system), X11 application still work good enough, thanks to Xwayland compatibility layer.
     */
    struct X11 {
        _unique<IPlatformAbstraction> operator()();
    };

    /**
     * @brief Uses gtk4 as core platform API.
     * @details
     * Default option on non-GNOME desktop environments.
     */
    struct Gtk4 {
        _unique<IPlatformAbstraction> operator()();
    };

    /**
     * @brief Uses libadwaita as core platform API, which is based on Gtk4.
     * @details
     * Default option on GNOME desktop environments.
     */
    struct Adwaita1 {
        _unique<IPlatformAbstraction> operator()();
    };

    using InitializationVariant = std::function<_unique<IPlatformAbstraction>()>;

    /**
     * @brief Defines APIs to try to use (first is prioritized).
     * @details
     * Defines a list of APIs to use. If the first API fails to initialize, the next one is probed, and so on, until
     * a successful API initialization is found.
     *
     * By default, this field is initialized as such:
     * <!-- aui:snippet aui.views/src/AUI/Platform/linux/APlatformAbstractionOptions.cpp APlatformAbstractionOptions::defaultInitializationOrder -->
     */
    AVector<InitializationVariant> initializationOrder = defaultInitializationOrder();

    static void set(APlatformAbstractionOptions options) {
        inst() = std::move(options);
    }
    static const APlatformAbstractionOptions& get() noexcept {
        return inst();
    }

    static AVector<InitializationVariant> defaultInitializationOrder();

private:
    static APlatformAbstractionOptions& inst();
};
