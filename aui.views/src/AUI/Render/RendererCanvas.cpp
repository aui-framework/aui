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
#include <AUI/Render/FontAtlas.hpp>
#include <range/v3/all.hpp>
#include <AUI/GL/OpenGLBackend.hpp>
#include <AUI/Software/SoftwareRenderer.h>

RendererCanvas::RendererCanvas(ACanvas& canvas, IRendererBackend& backend) : mCanvas(canvas), mBackend(backend) {}

_<IRenderer::IMultiStringCanvas> RendererCanvas::newMultiStringCanvas(const AFontStyle& style) {
    auto entryData = aui::getFontEntryData(style, mBackend.getFontCache());
    return _new<aui::MultiStringCanvas>(mBackend, entryData, style);
}

_<IRenderer::IPrerenderedString> RendererCanvas::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    if (text.empty()) return nullptr;
    auto c = newMultiStringCanvas(fs);
    c->addString(position, text);
    return c->finalize();
}

void RendererCanvas::setColorForced(const AColor& color) {
    mColorMultiplier = color;
}

const AColor& RendererCanvas::getColor() const {
    return mColorMultiplier;
}

void RendererCanvas::setTransform(const glm::mat4& transform) {
    mCanvas.setTransform(transform);
}

void RendererCanvas::setTransformForced(const glm::mat4& transform) {
    mCanvas.setTransformForced(transform);
}

void RendererCanvas::setBlending(Blending blending) {
    mBlending = blending;
}

glm::mat4 RendererCanvas::getProjectionMatrix() const {
    return mBackend.getProjectionMatrix();
}

glm::mat4 RendererCanvas::getTransform() {
    return mCanvas.getTransform();
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
    mBackend.setAllowRenderToTexture(allowRenderToTexture);
}

bool RendererCanvas::allowRenderToTexture() const noexcept {
    return mBackend.allowRenderToTexture();
}

void RendererCanvas::setRenderScale(float render_scale) {
    mRenderScale = render_scale;
}

float RendererCanvas::getRenderScale() const noexcept {
    return mRenderScale;
}

void RendererCanvas::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Any> backdrops) {
    mCanvas.backdrops(position, size, backdrops);
}

void RendererCanvas::stub(glm::vec2 position, glm::vec2 size) {
    mCanvas.rectangle(paint(ASolidBrush{0xa0a0a0_rgb}), position, size);
}
