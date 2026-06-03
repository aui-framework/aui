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

#include <AUI/Render/IRendererBackend.h>
#include <AUI/Render/ADisplayList.h>
#include <AUI/Render/ADisplayListCanvas.hpp>
#include <AUI/Render/RendererCanvas.h>

class CommonOffscreenRenderPass : public IOffscreenRenderPass {
public:
    IRendererBackend& backend;
    _<ITexture> target;
    ADisplayList displayList;
    ADisplayListCanvas canvas;
    RendererCanvas rendererCanvas;

    CommonOffscreenRenderPass(IRendererBackend& backend, const _<ITexture>& target) : 
        backend(backend), target(target), canvas(displayList, backend), rendererCanvas(canvas, backend) {}

    ARenderContext context() const override {
        return { const_cast<ADisplayListCanvas&>(canvas), backend, const_cast<RendererCanvas&>(rendererCanvas) };
    }
};
