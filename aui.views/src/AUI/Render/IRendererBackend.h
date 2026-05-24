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

#include <cstdint>
#include <span>
#include <glm/glm.hpp>
#include <AUI/Common/AColor.h>
#include <AUI/Render/ADisplayList.h>
#include "IRenderer.h"

class ASurface;

/**
 * @brief Actual renderer interface.
 */
class API_AUI_VIEWS IRendererBackend {
public:
    virtual ~IRendererBackend() = default;

    virtual _<ITexture> getNewTexture() = 0;
    virtual _unique<ITexture> createNewTexture() = 0;
    virtual _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) = 0;

    virtual void rectangle(const ADisplayList::Rectangle& v, const APaint& paint) = 0;
    virtual void roundedRectangle(const ADisplayList::RoundedRectangle& v, const APaint& paint) = 0;
    virtual void rectangleBorder(const ADisplayList::RectangleBorder& v, const APaint& paint) = 0;
    virtual void roundedRectangleBorder(const ADisplayList::RoundedRectangleBorder& v, const APaint& paint) = 0;
    virtual void boxShadow(const ADisplayList::BoxShadow& v, const APaint& paint) = 0;
    virtual void boxShadowInner(const ADisplayList::BoxShadowInner& v, const APaint& paint) = 0;
    virtual void string(const ADisplayList::Text& v, const APaint& paint) = 0;
    virtual _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) = 0;
    virtual void lines(const ADisplayList::Lines& v, const APaint& paint) = 0;
    virtual void points(const ADisplayList::Points& v, const APaint& paint) = 0;
    virtual void lines(const ADisplayList::LineBatches& v, const APaint& paint) = 0;
    virtual void squareSector(const ADisplayList::SquareSector& v, const APaint& paint) = 0;

    virtual void setTransformForced(const glm::mat4& transform) = 0;
    virtual const glm::mat4& getTransform() const = 0;
    virtual void setColorForced(const AColor& color) = 0;
    virtual const AColor& getColor() const = 0;
    virtual void setBlending(Blending blending) = 0;

    virtual void pushMaskBefore() = 0;
    virtual void pushMaskAfter() = 0;
    virtual void popMaskBefore() = 0;
    virtual void popMaskAfter() = 0;

    virtual _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept = 0;
    virtual void setWindow(ASurface* window) = 0;
    virtual ASurface* getWindow() const noexcept = 0;
    virtual glm::mat4 getProjectionMatrix() const = 0;

    virtual std::uint8_t getStencilDepth() const noexcept = 0;
    virtual void setStencilDepth(std::uint8_t stencilDepth) = 0;

    virtual float getRenderScale() const noexcept = 0;
    virtual void setRenderScale(float renderScale) = 0;
    virtual void setAllowRenderToTexture(bool allow) = 0;
    virtual bool allowRenderToTexture() const noexcept = 0;

    virtual void backdrops(const ADisplayList::Backdrop& v, const APaint& paint);
    virtual void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) = 0;
};
