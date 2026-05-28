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
#include <AUI/Render/IRendererInterfaces.h>
#include "IRenderer.h"

class ASurface;
namespace aui { class FontAtlas; class AFontCache; }

/**
 * @brief Actual renderer interface.
 * Stateless batch-oriented rendering backend.
 */
class API_AUI_VIEWS IRendererBackend {
public:
    virtual ~IRendererBackend() = default;

    virtual _<ITexture> createTexture(glm::u32vec2 size, APixelFormat format = APixelFormat::RGBA_BYTE) = 0;
    virtual _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) = 0;

    virtual void solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) = 0;
    virtual void lines(const ADisplayList::Lines& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void points(const ADisplayList::Points& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, const APaint& paint) = 0;
    virtual void squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, const APaint& paint) = 0;

    virtual _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept = 0;
    virtual void setWindow(ASurface* window) = 0;
    virtual ASurface* getWindow() const noexcept = 0;
    virtual glm::mat4 getProjectionMatrix() const = 0;

    virtual void setAllowRenderToTexture(bool allow) = 0;
    virtual bool allowRenderToTexture() const noexcept = 0;

    virtual void backdrops(const ADisplayList::Backdrop& v, const glm::mat4& transform);
    virtual void backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) = 0;

    virtual const _<aui::AFontCache>& getFontCache() = 0;
};
