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
#include <AUI/Render/ADrawList.hpp>
#include <AUI/Render/IRendererInterfaces.h>
#include "IRenderer.h"
#include <AUI/Render/ARenderContext.h>

class ASurface;
namespace aui { class FontAtlas; class AFontCache; }

class ACanvas;

class IOffscreenRenderPass {
public:
    virtual ~IOffscreenRenderPass() = default;
    virtual ARenderContext context() = 0;
};

/**
 * @brief Actual renderer interface.
 * Stateless batch-oriented rendering backend.
 */
class API_AUI_VIEWS IRendererBackend {
public:
    virtual ~IRendererBackend() = default;

    virtual _unique<IOffscreenRenderPass> beginOffscreen(const _<ITexture>& renderTarget) = 0;
    virtual void endOffscreen(_unique<IOffscreenRenderPass> pass) = 0;

    virtual _<ITexture> createTexture(glm::u32vec2 size, APixelFormat format = APixelFormat::R8G8B8A8_UNORM, TextureFilter filter = TextureFilter::LINEAR) = 0;
    virtual _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) = 0;

    virtual void solidRectangles(const ADrawList::SolidRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void gradientRectangles(const ADrawList::GradientRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void texturedRectangles(const ADrawList::TexturedRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void solidRoundedRectangles(const ADrawList::SolidRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void gradientRoundedRectangles(const ADrawList::GradientRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void texturedRoundedRectangles(const ADrawList::TexturedRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void rectangleBorders(const ADrawList::RectangleBorders& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void roundedRectangleBorders(const ADrawList::RoundedRectangleBorders& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void boxShadow(const ADrawList::BoxShadow& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void boxShadowInner(const ADrawList::BoxShadowInner& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void glyphs(const ADrawList::Glyphs& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) = 0;
    virtual void lines(const ADrawList::Lines& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void points(const ADrawList::Points& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void lines(const ADrawList::LineBatches& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void squareSector(const ADrawList::SquareSector& v, const glm::mat4& transform, const APaint& paint) = 0;

    virtual void setRenderTarget(const _<ITexture>& texture, glm::uvec2 size) = 0;
    virtual void setClipRect(const ARect<float>& rect) = 0;
    virtual glm::uvec2 getViewportSize() const = 0;
    virtual void setRenderMaskMode(bool enabled) = 0;
    virtual void clear(const AColor& color) = 0;

    virtual void beginRenderPass(const _<ITexture>& target) = 0;
    virtual void endRenderPass() = 0;
    virtual void flush() = 0;

    virtual glm::mat4 getProjectionMatrix() const = 0;

    virtual void setAllowRenderToTexture(bool allow) = 0;
    virtual bool allowRenderToTexture() const noexcept = 0;

    virtual void backdrops(const ADrawList::Backdrop& v, const glm::mat4& transform);
    virtual void backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) = 0;

    virtual void setMask(const _<ITexture>& mask, const glm::vec4& maskRect = glm::vec4(0.f)) = 0;

    struct AMergedMask {
        _<ITexture> texture;
        glm::vec4 rect;
    };
    virtual AMergedMask mergeMasks(const _<ITexture>& mask1, const glm::vec4& mask1Rect,
                                   const _<ITexture>& mask2, const glm::vec4& mask2Rect) = 0;

    virtual _<ITexture> createRectMask(const ARect<float>& rect, bool inverted, const ARect<float>& bounds) = 0;
    virtual _<ITexture> createRoundedRectMask(const ARect<float>& rect, float radius, bool inverted, const ARect<float>& bounds) = 0;

    virtual const _<aui::AFontCache>& getFontCache() = 0;
};
