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


#include "AWindowNativePtr.h"
#include "IRenderingContext.h"
#include <AUI/Common/AString.h>
#include <AUI/Enum/WindowStyle.h>
#include <AUI/Image/AImage.h>

class ASurface;
class AWindow;
class IRenderer;

/**
 * @brief Glue between AWindow and IRenderer.
 * @details
 * IRenderingContext is linker object between AWindow and IRenderer, that exposes IRenderer to the client code in order
 * to draw.
 *
 * Unlike IRenderer, each AWindow has it's own instance of IRenderingContext, thus it can be used to carry
 * platform-specific and window-specific objects, like handles and framebuffers.
 *
 * @sa IRenderer
 */
class API_AUI_VIEWS IRenderingContext {
public:
    struct API_AUI_VIEWS Init {
        AWindow& window;
        AString name;
        int width;
        int height;
        WindowStyle ws;
        AWindow* parent;

        void setRenderingContext(_unique<IRenderingContext>&& context) const;
    };
    virtual void init(const Init& init);
    virtual ~IRenderingContext() = default;
    virtual void destroyNativeWindow(ASurface& window) = 0;

    virtual AImage makeScreenshot() = 0;

    virtual void beginPaint(ASurface& window) = 0;
    virtual void endPaint(ASurface& window) = 0;
    virtual void beginResize(ASurface& window) = 0;
    virtual void endResize(ASurface& window) = 0;

    virtual IRenderer& renderer() = 0;
};