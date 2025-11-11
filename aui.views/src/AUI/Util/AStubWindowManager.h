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
#include "AUI/Software/SoftwareRenderer.h"

/**
 * @brief Window manager used to stub the default window manager.
 * @ingroup views
 * @details
 * Useful for command-line only use-cases where the actual windows are not viable (i.e. tests).
 * Rendering is switched to software when.
 *
 * ```cpp
 * window = _new<AWindow>("");
 * window->show();
 * ...
 * auto screenshot = AStubWindowManager::makeScreenshot(window);
 * ```
 */
class API_AUI_VIEWS AStubWindowManager: public AWindowManager {
public:
    friend class StubRenderingContext;

    void initNativeWindow(const IRenderingContext::Init& init) override;
    static AImage makeScreenshot(aui::no_escape<AWindow> window);
    static void drawFrame();

    /**
     * @brief Stub window manager configuration to narrow ui test/benchmark scope.
     */
    struct Config {
        /**
         * @brief Whether to call window paint.
         */
        bool paint = true;

        /**
         * @brief Whether to call layout engine.
         */
        bool layout = true;

        /**
         * @brief Renderer used for window manager.
         */
        _unique<IRenderer> renderer = std::make_unique<SoftwareRenderer>();
    };

    static void setConfig(Config config);

private:
    Config mConfig;

    void drawFrameImpl();
};
