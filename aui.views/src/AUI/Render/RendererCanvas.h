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

#include <AUI/Render/IRenderer.h>
#include <AUI/Render/ACanvas.hpp>

class API_AUI_VIEWS RendererCanvas : public IRenderer {
public:
    RendererCanvas(ACanvas& canvas);

    ACanvas& canvas() override { return mCanvas; }

    _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;

    void rectangle(const ABrush& brush,
                           glm::vec2 position,
                           glm::vec2 size) override;

    void roundedRectangle(const ABrush& brush,
                                  glm::vec2 position,
                                  glm::vec2 size,
                                  float radius) override;

    void rectangleBorder(const ABrush& brush,
                                 glm::vec2 position,
                                 glm::vec2 size,
                                 float lineWidth = 1.f) override;

    void roundedRectangleBorder(const ABrush& brush,
                                        glm::vec2 position,
                                        glm::vec2 size,
                                        float radius,
                                        int borderWidth) override;

    void boxShadow(glm::vec2 position,
                           glm::vec2 size,
                           float blurRadius,
                           const AColor& color) override;

    void boxShadowInner(glm::vec2 position,
                                glm::vec2 size,
                                float blurRadius,
                                float spreadRadius,
                                float borderRadius,
                                const AColor& color,
                                glm::vec2 offset) override;

    void string(glm::vec2 position,
                        const AString& string,
                        const AFontStyle& fs) override;

    _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    void line(const ABrush& brush, glm::vec2 p1, glm::vec2 p2, const ABorderStyle& style = ABorderStyle::Solid{}, AMetric width = 1_dp) override;

    void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) override;

    void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style = ABorderStyle::Solid{}) override;

    void points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) override;

    void lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) override;

    void lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style = ABorderStyle::Solid{}) override;

    void squareSector(const ABrush& brush,
                              const glm::vec2& position,
                              const glm::vec2& size,
                              AAngleRadians begin,
                              AAngleRadians end) override;

    void setColorForced(const AColor& color) override;

    const AColor& getColor() const override;

    void setTransform(const glm::mat4& transform) override;

    void setTransformForced(const glm::mat4& transform) override;

    void setBlending(Blending blending) override;

    glm::mat4 getProjectionMatrix() const override;

    glm::mat4 getTransform() override;

    void translate(const glm::vec2& offset) override;

    void rotate(const glm::vec3& axis, AAngleRadians angle) override;

    void rotate(AAngleRadians angle) override;

    void setAllowRenderToTexture(bool allowRenderToTexture) override;

    [[nodiscard]]
    bool allowRenderToTexture() const noexcept override;

    void setRenderScale(float render_scale) override;

    float getRenderScale() const noexcept override;

    void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Any> backdrops) override;

    void stub(glm::vec2 position, glm::vec2 size) override;

private:
    APaint paint(const ABrush& brush) const {
        return {brush, mColorMultiplier, mBlending, 1.0f};
    }

    ACanvas& mCanvas;
    Blending mBlending = Blending::NORMAL;
    AColor mColorMultiplier = AColor::WHITE;
};
