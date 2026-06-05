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

#include <glm/glm.hpp>
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Render/ADisplayList.h>

class API_AUI_VIEWS ADisplayListCanvas : public ACanvas {
public:
    ADisplayListCanvas(ADisplayList& displayList, IRendererBackend& renderer) : mDisplayList(displayList), mRenderer(renderer) {}

    _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;
    _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    size_t save() override;
    void restore() override;
    void restore(size_t targetStackSize) override;

    void pushClipRect(const ARect<float>& rect, AClipOp op) override;
    void popClipRect() override;

    void pushLayer() override;
    void popLayer() override;
    void pushMask(const _<ITexture>& maskTexture, const glm::vec4& maskRect) override;
    void popMask() override;

    void clear(const AColor& color) override;

    void setTransform(const glm::mat4& transform) override;

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
    void glyphRect(const _<ITexture>& texture, glm::vec2 position, glm::vec2 size, glm::vec2 u1, glm::vec2 u2, const AColor& color, bool isSubpixel) override;
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

    void scale(const glm::vec2& multiplier) override;

    const glm::mat4 getTransform() const noexcept override { return mBaseTransform * mTransform; }
    void setTransformForced(const glm::mat4& transform) noexcept override;

    void translate(const glm::vec2& offset) override;
    void rotate(const glm::vec3& axis, AAngleRadians angle) override;
    void rotate(AAngleRadians angle) override;

    const glm::mat4& getBaseTransform() const noexcept override { return mBaseTransform; }
    void setBaseTransform(const glm::mat4& transform) noexcept override { mBaseTransform = transform; }

private:
    void add(ADisplayList::StoredCommand::Command command, const APaint& paint = {});

    struct State {
        glm::mat4 transform;
        glm::mat4 baseTransform;
        size_t maskStackDepth;
        size_t layerStackDepth;
        size_t clipStackDepth;
    };
    std::vector<State> mStates;
    glm::mat4 mTransform = glm::mat4(1.0f);
    glm::mat4 mBaseTransform = glm::mat4(1.0f);
    size_t mMaskStackDepth = 0;
    size_t mLayerStackDepth = 0;
    size_t mClipStackDepth = 0;

    ADisplayList& mDisplayList;
    IRendererBackend& mRenderer;
};
