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


#include "IRenderingContext.h"
#include "ASurface.h"

/**
 * Fake window initializer which does not create any window. Applicable in UI tests
 */
class AFakeWindowInitializer: public IRenderingContext {
public:
    void init(const Init& init);

    ~AFakeWindowInitializer() override = default;

    void beginPaint(ASurface& window) override;

    void endPaint(ASurface& window) override;

    void beginResize(ASurface& window) override;

    void destroyNativeWindow(ASurface& window) override;
};


