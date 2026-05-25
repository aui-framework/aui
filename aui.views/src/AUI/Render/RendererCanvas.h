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

    ACanvas& getCanvas() const { return mCanvas; }

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
                        const AFontStyle& fs = {}) override;

    _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) override;

    void points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) override;

    void lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) override;

    void squareSector(const ABrush& brush,
                              const glm::vec2& position,
                              const glm::vec2& size,
                              AAngleRadians begin,
                              AAngleRadians end) override;

    void setBlending(Blending blending) override;

    void pushMaskBefore() override;

    void pushMaskAfter() override;

    void popMaskBefore() override;

    void popMaskAfter() override;

    _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept override;

    glm::mat4 getProjectionMatrix() const override;

protected:
    _unique<ITexture> createNewTexture() override;

private:
    ACanvas& mCanvas;

    void sync();
};
