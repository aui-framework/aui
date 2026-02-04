// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

//
// Created by alex2772 on 2/3/26.
//

#include "IBatchingRenderer.h"

void IBatchingRenderer::rectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size) {
    mCmds.emplace_back(CmdRectangle{
        .brush = std::move(brush),
        .position = position,
        .size = size,
    });
}

void IBatchingRenderer::roundedRectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size, float radius) {
    mCmds.emplace_back(CmdRoundedRectangle{
        .brush = brush,
        .position = position,
        .size = size,
        .radius = radius,
    });
}

void IBatchingRenderer::rectangleBorder(const ABrush& brush, glm::vec2 position, glm::vec2 size, float lineWidth) {
    mCmds.emplace_back(CmdRectangleBorder{
        .brush = brush,
        .position = position,
        .size = size,
        .lineWidth = lineWidth,
    });
}

void IBatchingRenderer::roundedRectangleBorder(const ABrush& brush, glm::vec2 position, glm::vec2 size, float radius, int borderWidth) {
    mCmds.emplace_back(CmdRoundedRectangleBorder{
        .brush = brush,
        .position = position,
        .size = size,
        .radius = radius,
        .borderWidth = borderWidth,
    });
}

void IBatchingRenderer::boxShadow(glm::vec2 position, glm::vec2 size, float blurRadius, const AColor& color) {
    mCmds.emplace_back(CmdBoxShadow{
        .position = position,
        .size = size,
        .blurRadius = blurRadius,
        .color = color,
    });
}

void IBatchingRenderer::boxShadowInner(glm::vec2 position, glm::vec2 size, float blurRadius, float spreadRadius, float borderRadius, const AColor& color, glm::vec2 offset) {
    mCmds.emplace_back(CmdBoxShadowInner{
        .position = position,
        .size = size,
        .blurRadius = blurRadius,
        .spreadRadius = spreadRadius,
        .borderRadius = borderRadius,
        .color = color,
        .offset = offset,
    });
}

void IBatchingRenderer::string(glm::vec2 position, const AString& string, const AFontStyle& fs) {
    mCmds.emplace_back(CmdString{
        .position = position,
        .string = string,
        .fs = fs,
    });
}


void IBatchingRenderer::lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) {
    mCmds.emplace_back(CmdLines{
        .brush = brush,
        .points = points,
        .style = style,
        .width = width,
    });
}

void IBatchingRenderer::points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) {
    mCmds.emplace_back(CmdPoints{
        .brush = brush,
        .points = points,
        .size = size,
    });
}

void IBatchingRenderer::lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) {
    mCmds.emplace_back(CmdLinesPairs{
        .brush = brush,
        .points = points,
        .style = style,
        .width = width,
    });
}

void IBatchingRenderer::squareSector(const ABrush& brush, const glm::vec2& position, const glm::vec2& size, AAngleRadians begin, AAngleRadians end) {
    mCmds.emplace_back(CmdSquareSector{
        .brush = brush,
        .position = position,
        .size = size,
        .begin = begin,
        .end = end,
    });
}

void IBatchingRenderer::pushMaskBefore() {
    mCmds.emplace_back(CmdPushMask{});
}

void IBatchingRenderer::pushMaskAfter() {
    mCmds.emplace_back(CmdPushMask{});
}

void IBatchingRenderer::popMaskBefore() {
    mCmds.emplace_back(CmdPopMask{});
}

void IBatchingRenderer::popMaskAfter() {
    mCmds.emplace_back(CmdPopMask{});
}

void IBatchingRenderer::setBlending(Blending blending) {
    mCmds.emplace_back(CmdSetBlending{.blending = blending});
}

void IBatchingRenderer::setWindow(AWindowBase* window) {
    mCmds.emplace_back(CmdSetWindow{.window = window});
}

void IBatchingRenderer::flush() {
    handleCmds(std::exchange(mCmds, {}));
}
