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

#include "AUIRenderer.h"
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Render/ARender/IRendererBackend.h>
#include <AUI/Render/FontAtlas.hpp>
#include <range/v3/all.hpp>
#include <AUI/Render/ARender/GL/OpenGLBackend.hpp>
#include <AUI/Render/ARender/Software/SoftwareRenderer.h>

AUIRenderer::AUIRenderer(ACanvas& canvas, IRendererBackend& backend) : mCanvas(canvas), mBackend(backend) {}

_<IRenderer::IMultiStringCanvas> AUIRenderer::newMultiStringCanvas(const AFontStyle& style) {
    auto entryData = aui::getFontEntryData(style, mBackend.getFontCache());
    return _new<aui::MultiStringCanvas>(mBackend, entryData, style);
}

_<IRenderer::IPrerenderedString> AUIRenderer::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    if (text.empty()) return nullptr;
    auto c = newMultiStringCanvas(fs);
    c->addString(position, text);
    return c->finalize();
}

void AUIRenderer::setColorForced(const AColor& color) {
    mColorMultiplier = color;
}

const AColor& AUIRenderer::getColor() const {
    return mColorMultiplier;
}

void AUIRenderer::setTransform(const glm::mat4& transform) {
    mCanvas.setTransform(transform);
}

void AUIRenderer::setTransformForced(const glm::mat4& transform) {
    mCanvas.setTransformForced(transform);
}

void AUIRenderer::setBlending(Blending blending) {
    mBlending = blending;
}

glm::mat4 AUIRenderer::getProjectionMatrix() const {
    return mBackend.getProjectionMatrix();
}

glm::mat4 AUIRenderer::getTransform() {
    return mCanvas.getTransform();
}

void AUIRenderer::translate(const glm::vec2& offset) {
    mCanvas.translate(offset);
}

void AUIRenderer::rotate(const glm::vec3& axis, AAngleRadians angle) {
    mCanvas.rotate(axis, angle);
}

void AUIRenderer::rotate(AAngleRadians angle) {
    mCanvas.rotate(angle);
}

void AUIRenderer::setAllowRenderToTexture(bool allowRenderToTexture) {
    mBackend.setAllowRenderToTexture(allowRenderToTexture);
}

bool AUIRenderer::allowRenderToTexture() const noexcept {
    return mBackend.allowRenderToTexture();
}

void AUIRenderer::setRenderScale(float render_scale) {
    mRenderScale = render_scale;
}

float AUIRenderer::getRenderScale() const noexcept {
    return mRenderScale;
}

void AUIRenderer::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Any> backdrops) {
    mCanvas.backdrops(position, size, backdrops);
}

void AUIRenderer::stub(glm::vec2 position, glm::vec2 size) {
    mCanvas.rectangle(paint(ASolidBrush{0xa0a0a0_rgb}), position, size);
}
