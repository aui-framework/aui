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

#include "ADisplayListCanvas.hpp"
#include <AUI/Render/IRendererBackend.h>

#include <utility>

_<ITexture> ADisplayListCanvas::getNewTexture() {
    return mRenderer.getNewTexture();
}

_unique<ITexture> ADisplayListCanvas::createNewTexture() {
    return mRenderer.createNewTexture();
}

float ADisplayListCanvas::getRenderScale() const noexcept {
    return mRenderer.getRenderScale();
}

_<IRenderer::IMultiStringCanvas> ADisplayListCanvas::newMultiStringCanvas(const AFontStyle& style) {
    return mRenderer.newMultiStringCanvas(style);
}

_<IRenderer::IPrerenderedString> ADisplayListCanvas::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    return mRenderer.prerenderString(position, text, fs);
}

void ADisplayListCanvas::pushLayer() { add(ADisplayList::PushLayer{}); }

void ADisplayListCanvas::popLayer() { add(ADisplayList::PopLayer{}); }

void ADisplayListCanvas::rectangle(const APaint& paint, glm::vec2 position, glm::vec2 size) {
    add(ADisplayList::Rectangle{position, size}, paint);
}

void ADisplayListCanvas::roundedRectangle(const APaint& paint, glm::vec2 position, glm::vec2 size, float radius) {
    add(ADisplayList::RoundedRectangle{position, size, radius}, paint);
}

void ADisplayListCanvas::rectangleBorder(const APaint& paint, glm::vec2 position, glm::vec2 size, float lineWidth) {
    add(ADisplayList::RectangleBorder{position, size, lineWidth}, paint);
}

void ADisplayListCanvas::roundedRectangleBorder(const APaint& paint,
                                                glm::vec2 position,
                                                glm::vec2 size,
                                                float radius,
                                                int borderWidth) {
    add(ADisplayList::RoundedRectangleBorder{position, size, radius, borderWidth}, paint);
}

void ADisplayListCanvas::boxShadow(const APaint& paint, glm::vec2 position, glm::vec2 size, float blurRadius, const AColor& color) {
    add(ADisplayList::BoxShadow{position, size, blurRadius, color}, paint);
}

void ADisplayListCanvas::boxShadowInner(const APaint& paint,
                                        glm::vec2 position,
                                        glm::vec2 size,
                                        float blurRadius,
                                        float spreadRadius,
                                        float borderRadius,
                                        const AColor& color,
                                        glm::vec2 offset) {
    add(ADisplayList::BoxShadowInner{position, size, blurRadius, spreadRadius, borderRadius, color, offset}, paint);
}

void ADisplayListCanvas::string(const APaint& paint, glm::vec2 position, const AString& string, const AFontStyle& fs) {
    add(ADisplayList::Text{position, string, fs}, paint);
}

void ADisplayListCanvas::prerenderedString(const APaint& paint, glm::vec2 position, const _<IRenderer::IPrerenderedString>& prerenderedString) {
    add(ADisplayList::PrerenderedString{position, prerenderedString}, paint);
}

void ADisplayListCanvas::lines(const APaint& paint, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) {
    add(ADisplayList::Lines{{points.begin(), points.end()}, style, width}, paint);
}

void ADisplayListCanvas::points(const APaint& paint, AArrayView<glm::vec2> points, AMetric size) {
    add(ADisplayList::Points{{points.begin(), points.end()}, size}, paint);
}

void ADisplayListCanvas::lines(const APaint& paint,
                               AArrayView<std::pair<glm::vec2, glm::vec2>> points,
                               const ABorderStyle& style,
                               AMetric width) {
    add(ADisplayList::LineBatches{{points.begin(), points.end()}, style, width}, paint);
}

void ADisplayListCanvas::squareSector(const APaint& paint,
                                      const glm::vec2& position,
                                      const glm::vec2& size,
                                      AAngleRadians begin,
                                      AAngleRadians end) {
    add(ADisplayList::SquareSector{position, size, begin, end}, paint);
}

void ADisplayListCanvas::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Any> backdrops) {
    add(ADisplayList::Backdrop{position, size, {backdrops.begin(), backdrops.end()}});
}

void ADisplayListCanvas::pushMaskBefore() { add(ADisplayList::MaskBefore{}); }

void ADisplayListCanvas::pushMaskAfter() {
    add(ADisplayList::MaskAfter{});
    mStencilDepth++;
}

void ADisplayListCanvas::popMaskBefore() { add(ADisplayList::PopMaskBefore{}); }

void ADisplayListCanvas::popMaskAfter() {
    add(ADisplayList::PopMaskAfter{});
    if (mStencilDepth > 0) {
        mStencilDepth--;
    }
}

void ADisplayListCanvas::add(ADisplayList::StoredCommand::Command command, const APaint& paint) {
    APaint combined = paint;
    combined.color *= mColorMultiplier;
    combined.opacity *= mOpacity;
    combined.blending = mBlending;
    mDisplayList.add(std::move(command), getTransform(), combined, getStencilDepth());
}
