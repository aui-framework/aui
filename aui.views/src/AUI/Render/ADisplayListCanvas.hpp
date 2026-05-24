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

#include <AUI/Render/ACanvas.hpp>
#include <AUI/Render/ADisplayList.h>

class IRendererBackend;

class ADisplayListCanvas: public ACanvas {
public:
    ADisplayListCanvas(ADisplayList& displayList, IRendererBackend& renderer) : mDisplayList(displayList), mRenderer(renderer) {}

    _<ITexture> getNewTexture() override;
    _unique<ITexture> createNewTexture() override;

    float getRenderScale() const noexcept override;
    _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;
    _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    void pushLayer() override;
    void popLayer() override;
    void rectangle(const APaint& paint, glm::vec2 position, glm::vec2 size) override;
    void roundedRectangle(const APaint& paint, glm::vec2 position, glm::vec2 size, float radius) override;
    void rectangleBorder(const APaint& paint, glm::vec2 position, glm::vec2 size, float lineWidth = 1.f) override;
    void roundedRectangleBorder(const APaint& paint, glm::vec2 position, glm::vec2 size, float radius, int borderWidth) override;
    void boxShadow(const APaint& paint, glm::vec2 position, glm::vec2 size, float blurRadius, const AColor& color) override;
    void boxShadowInner(const APaint& paint,
                        glm::vec2 position,
                        glm::vec2 size,
                        float blurRadius,
                        float spreadRadius,
                        float borderRadius,
                        const AColor& color,
                        glm::vec2 offset) override;
    void string(const APaint& paint, glm::vec2 position, const AString& string, const AFontStyle& fs = {}) override;
    void prerenderedString(const APaint& paint, glm::vec2 position, const _<IRenderer::IPrerenderedString>& prerenderedString) override;
    void lines(const APaint& paint, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) override;
    void points(const APaint& paint, AArrayView<glm::vec2> points, AMetric size) override;
    void lines(const APaint& paint,
               AArrayView<std::pair<glm::vec2, glm::vec2>> points,
               const ABorderStyle& style,
               AMetric width) override;
    void squareSector(const APaint& paint,
                      const glm::vec2& position,
                      const glm::vec2& size,
                      AAngleRadians begin,
                      AAngleRadians end) override;
    void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Any> backdrops) override;
    void pushMaskBefore() override;
    void pushMaskAfter() override;
    void popMaskBefore() override;
    void popMaskAfter() override;

private:
    void add(ADisplayList::StoredCommand::Command command, const APaint& paint = {});

    ADisplayList& mDisplayList;
    IRendererBackend& mRenderer;
};
