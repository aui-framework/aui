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

RendererCanvas::RendererCanvas(ACanvas& canvas) : mCanvas(canvas) {}

_<IRenderer::IMultiStringCanvas> RendererCanvas::newMultiStringCanvas(const AFontStyle& style) {
    return mCanvas.newMultiStringCanvas(style);
}

void RendererCanvas::rectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size) {
    sync();
    mCanvas.rectangle({brush}, position, size);
}

void RendererCanvas::roundedRectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size, float radius) {
    sync();
    mCanvas.roundedRectangle({brush}, position, size, radius);
}

void RendererCanvas::rectangleBorder(const ABrush& brush, glm::vec2 position, glm::vec2 size, float lineWidth) {
    sync();
    mCanvas.rectangleBorder({brush}, position, size, lineWidth);
}

void RendererCanvas::roundedRectangleBorder(const ABrush& brush,
                                            glm::vec2 position,
                                            glm::vec2 size,
                                            float radius,
                                            int borderWidth) {
    sync();
    mCanvas.roundedRectangleBorder({brush}, position, size, radius, borderWidth);
}

void RendererCanvas::boxShadow(glm::vec2 position, glm::vec2 size, float blurRadius, const AColor& color) {
    sync();
    mCanvas.boxShadow({}, position, size, blurRadius, color);
}

void RendererCanvas::boxShadowInner(glm::vec2 position,
                                    glm::vec2 size,
                                    float blurRadius,
                                    float spreadRadius,
                                    float borderRadius,
                                    const AColor& color,
                                    glm::vec2 offset) {
    sync();
    mCanvas.boxShadowInner({}, position, size, blurRadius, spreadRadius, borderRadius, color, offset);
}

void RendererCanvas::string(glm::vec2 position, const AString& string, const AFontStyle& fs) {
    sync();
    mCanvas.string({}, position, string, fs);
}

_<IRenderer::IPrerenderedString> RendererCanvas::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    return mCanvas.prerenderString(position, text, fs);
}

void RendererCanvas::lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) {
    sync();
    mCanvas.lines({brush}, points, style, width);
}

void RendererCanvas::points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) {
    sync();
    mCanvas.points({brush}, points, size);
}

void RendererCanvas::lines(const ABrush& brush,
                           AArrayView<std::pair<glm::vec2, glm::vec2>> points,
                           const ABorderStyle& style,
                           AMetric width) {
    sync();
    mCanvas.lines({brush}, points, style, width);
}

void RendererCanvas::squareSector(const ABrush& brush,
                                  const glm::vec2& position,
                                  const glm::vec2& size,
                                  AAngleRadians begin,
                                  AAngleRadians end) {
    sync();
    mCanvas.squareSector({brush}, position, size, begin, end);
}

void RendererCanvas::setBlending(Blending blending) {
    mCanvas.setBlending(blending);
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

_unique<IRenderViewToTexture> RendererCanvas::newRenderViewToTexture() noexcept {
    return nullptr;
}

glm::mat4 RendererCanvas::getProjectionMatrix() const {
    return glm::mat4(1.0f);
}

void RendererCanvas::sync() {
    mCanvas.setTransformForced(mTransform);
    mCanvas.setBaseTransform(getProjectionMatrix());
    mCanvas.setColorMultiplier(mColor);
    mCanvas.setStencilDepth(mStencilDepth);
}
