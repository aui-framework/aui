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

#include "RendererCanvas.h"
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Render/IRendererBackend.h>
#include <range/v3/all.hpp>

RendererCanvas::RendererCanvas(ACanvas& canvas) : mCanvas(canvas) {}

_<IRenderer::IMultiStringCanvas> RendererCanvas::newMultiStringCanvas(const AFontStyle& style) {
    return mCanvas.newMultiStringCanvas(style);
}

void RendererCanvas::rectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size) {
    mCanvas.rectangle({brush}, position, size);
}

void RendererCanvas::roundedRectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size, float radius) {
    mCanvas.roundedRectangle({brush}, position, size, radius);
}

void RendererCanvas::rectangleBorder(const ABrush& brush, glm::vec2 position, glm::vec2 size, float lineWidth) {
    mCanvas.rectangleBorder({brush}, position, size, lineWidth);
}

void RendererCanvas::roundedRectangleBorder(const ABrush& brush,
                                            glm::vec2 position,
                                            glm::vec2 size,
                                            float radius,
                                            int borderWidth) {
    mCanvas.roundedRectangleBorder({brush}, position, size, radius, borderWidth);
}

void RendererCanvas::boxShadow(glm::vec2 position, glm::vec2 size, float blurRadius, const AColor& color) {
    mCanvas.boxShadow({}, position, size, blurRadius, color);
}

void RendererCanvas::boxShadowInner(glm::vec2 position,
                                    glm::vec2 size,
                                    float blurRadius,
                                    float spreadRadius,
                                    float borderRadius,
                                    const AColor& color,
                                    glm::vec2 offset) {
    mCanvas.boxShadowInner({}, position, size, blurRadius, spreadRadius, borderRadius, color, offset);
}

void RendererCanvas::string(glm::vec2 position, const AString& string, const AFontStyle& fs) {
    mCanvas.string({}, position, string, fs);
}

_<IRenderer::IPrerenderedString> RendererCanvas::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    return mCanvas.prerenderString(position, text, fs);
}

void RendererCanvas::line(const ABrush& brush, glm::vec2 p1, glm::vec2 p2, const ABorderStyle& style, AMetric width) {
    glm::vec2 points[] = { p1, p2 };
    lines(brush, points, style, width);
}

void RendererCanvas::lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) {
    mCanvas.lines({brush}, points, style, width);
}

void RendererCanvas::lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style) {
    lines(brush, points, style, 1_dp);
}

void RendererCanvas::points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) {
    mCanvas.points({brush}, points, size);
}

void RendererCanvas::lines(const ABrush& brush,
                           AArrayView<std::pair<glm::vec2, glm::vec2>> points,
                           const ABorderStyle& style,
                           AMetric width) {
    mCanvas.lines({brush}, points, style, width);
}

void RendererCanvas::lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style) {
    lines(brush, points, style, 1_dp);
}

void RendererCanvas::squareSector(const ABrush& brush,
                                  const glm::vec2& position,
                                  const glm::vec2& size,
                                  AAngleRadians begin,
                                  AAngleRadians end) {
    mCanvas.squareSector({brush}, position, size, begin, end);
}

void RendererCanvas::setColorForced(const AColor& color) {
    mCanvas.setColorForced(color);
}

void RendererCanvas::setColor(const AColor& color) {
    mCanvas.setColor(color);
}

const AColor& RendererCanvas::getColor() const {
    return mCanvas.getColor();
}

void RendererCanvas::setTransform(const glm::mat4& transform) {
    mCanvas.setTransform(transform);
}

void RendererCanvas::setTransformForced(const glm::mat4& transform) {
    mCanvas.setTransformForced(transform);
}

void RendererCanvas::pushMaskBefore() {
    mCanvas.pushMaskBefore();
}

void RendererCanvas::pushMaskAfter() {
    mCanvas.pushMaskAfter();
}

void RendererCanvas::popMaskBefore() {
    mCanvas.popMaskBefore();
}

void RendererCanvas::popMaskAfter() {
    mCanvas.popMaskAfter();
}

void RendererCanvas::setBlending(Blending blending) {
    mCanvas.setBlending(blending);
}

_unique<IRenderViewToTexture> RendererCanvas::newRenderViewToTexture() noexcept {
    return mCanvas.renderer().newRenderViewToTexture();
}

void RendererCanvas::setWindow(ASurface* window) {
    mWindow = window;
    mCanvas.renderer().setWindow(window);
}

ASurface* RendererCanvas::getWindow() const noexcept {
    return mWindow;
}

glm::mat4 RendererCanvas::getProjectionMatrix() const {
    return mCanvas.renderer().getProjectionMatrix();
}

glm::mat4 RendererCanvas::getTransform() {
    return mCanvas.getTransform();
}

std::uint8_t RendererCanvas::getStencilDepth() const noexcept {
    return mCanvas.getStencilDepth();
}

void RendererCanvas::setStencilDepth(uint8_t stencilDepth) {
    mCanvas.setStencilDepth(stencilDepth);
}

void RendererCanvas::translate(const glm::vec2& offset) {
    mCanvas.translate(offset);
}

void RendererCanvas::rotate(const glm::vec3& axis, AAngleRadians angle) {
    mCanvas.rotate(axis, angle);
}

void RendererCanvas::rotate(AAngleRadians angle) {
    mCanvas.rotate(angle);
}

void RendererCanvas::setAllowRenderToTexture(bool allowRenderToTexture) {
    mCanvas.renderer().setAllowRenderToTexture(allowRenderToTexture);
}

bool RendererCanvas::allowRenderToTexture() const noexcept {
    return mCanvas.renderer().allowRenderToTexture();
}

void RendererCanvas::setRenderScale(float render_scale) {
    mCanvas.renderer().setRenderScale(render_scale);
}

float RendererCanvas::getRenderScale() const noexcept {
    return mCanvas.renderer().getRenderScale();
}

void RendererCanvas::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<ass::Backdrop::Any> backdrops) {
    using Preprocessed = ass::Backdrop::Preprocessed;
    auto preprocessed =
        backdrops | ranges::views::transform([](const ass::Backdrop::Any& val) -> Preprocessed {
            return std::visit(
                aui::lambda_overloaded {
                  [](const ass::Backdrop::GaussianBlur& b) -> Preprocessed {
                      return b.findOptimalParams();
                  },
                  [](const auto& b) -> Preprocessed { return b; },
                },
                val);
        }) |
        ranges::to_vector;
    this->backdrops(position, size, std::span<const Preprocessed>(preprocessed.data(), preprocessed.size()));
}

void RendererCanvas::stub(glm::vec2 position, glm::vec2 size) {
    rectangle(ASolidBrush{0xa0a0a0_rgb}, position, size);
}

void RendererCanvas::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) {
    stub(position, size);
}
