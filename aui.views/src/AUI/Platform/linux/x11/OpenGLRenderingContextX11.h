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

#include <AUI/Platform/OpenGLRenderingContext.h>
#include <AUI/Platform/linux/x11/RenderingContextX11.h>
#include <GL/glx.h>

class OpenGLRenderingContextX11: public OpenGLRenderingContext, public RenderingContextX11 {
public:
    using OpenGLRenderingContext::OpenGLRenderingContext;

    ~OpenGLRenderingContextX11() override = default;
    void init(const Init& init) override;
    void destroyNativeWindow(ASurface& window) override;
    void beginPaint(ASurface& window) override;
    void endPaint(ASurface& window) override;
    void beginResize(ASurface& window) override;

private:
    static GLXContext ourContext;
};
