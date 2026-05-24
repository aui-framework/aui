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

#include "CanvasRenderer.h"
#include <AUI/Render/ACanvas.hpp>

CanvasRenderer::CanvasRenderer(ACanvas& canvas) : mCanvas(canvas) {}

_<IRenderer::IMultiStringCanvas> CanvasRenderer::newMultiStringCanvas(const AFontStyle& style) {
    return mCanvas.newMultiStringCanvas(style);
}

void CanvasRenderer::rectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size) {
    sync();
    mCanvas.rectangle({brush}, position, size);
}

void CanvasRenderer::roundedRectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size, float radius) {
    sync();
    mCanvas.roundedRectangle({brush}, position, size, radius);
}

void CanvasRenderer::rectangleBorder(const ABrush& brush, glm::vec2 position, glm::vec2 size, float lineWidth) {
    sync();
    mCanvas.rectangleBorder({brush}, position, size, lineWidth);
}

void CanvasRenderer::roundedRectangleBorder(const ABrush& brush,
                                            glm::vec2 position,
                                            glm::vec2 size,
                                            float radius,
                                            int borderWidth) {
    sync();
    mCanvas.roundedRectangleBorder({brush}, position, size, radius, borderWidth);
}

void CanvasRenderer::boxShadow(glm::vec2 position, glm::vec2 size, float blurRadius, const AColor& color) {
    sync();
    mCanvas.boxShadow({}, position, size, blurRadius, color);
}

void CanvasRenderer::boxShadowInner(glm::vec2 position,
                                    glm::vec2 size,
                                    float blurRadius,
                                    float spreadRadius,
                                    float borderRadius,
                                    const AColor& color,
                                    glm::vec2 offset) {
    sync();
    mCanvas.boxShadowInner({}, position, size, blurRadius, spreadRadius, borderRadius, color, offset);
}

void CanvasRenderer::string(glm::vec2 position, const AString& string, const AFontStyle& fs) {
    sync();
    mCanvas.string({}, position, string, fs);
}

_<IRenderer::IPrerenderedString> CanvasRenderer::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    return mCanvas.prerenderString(position, text, fs);
}

void CanvasRenderer::lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) {
    sync();
    mCanvas.lines({brush}, points, style, width);
}

void CanvasRenderer::points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) {
    sync();
    mCanvas.points({brush}, points, size);
}

void CanvasRenderer::lines(const ABrush& brush,
                           AArrayView<std::pair<glm::vec2, glm::vec2>> points,
                           const ABorderStyle& style,
                           AMetric width) {
    sync();
    mCanvas.lines({brush}, points, style, width);
}

void CanvasRenderer::squareSector(const ABrush& brush,
                                  const glm::vec2& position,
                                  const glm::vec2& size,
                                  AAngleRadians begin,
                                  AAngleRadians end) {
    sync();
    mCanvas.squareSector({brush}, position, size, begin, end);
}

void CanvasRenderer::setBlending(Blending blending) {
    mCanvas.setBlending(blending);
}

void CanvasRenderer::pushMaskBefore() {
    mCanvas.pushMaskBefore();
}

void CanvasRenderer::pushMaskAfter() {
    mCanvas.pushMaskAfter();
}

void CanvasRenderer::popMaskBefore() {
    mCanvas.popMaskBefore();
}

void CanvasRenderer::popMaskAfter() {
    mCanvas.popMaskAfter();
}

_unique<IRenderViewToTexture> CanvasRenderer::newRenderViewToTexture() noexcept {
    return nullptr;
}

glm::mat4 CanvasRenderer::getProjectionMatrix() const {
    return glm::mat4(1.0f);
}

_unique<ITexture> CanvasRenderer::createNewTexture() {
    return mCanvas.createNewTexture();
}

void CanvasRenderer::sync() {
    mCanvas.setTransformForced(mTransform);
    mCanvas.setColorMultiplier(mColor);
    mCanvas.setStencilDepth(mStencilDepth);
}
