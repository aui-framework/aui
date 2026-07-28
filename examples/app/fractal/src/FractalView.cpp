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

//
// Created by alex2772 on 12/9/20.
//

#include "FractalView.h"
#include "AUI/Render/IRenderer.h"
#include <glm/gtc/matrix_transform.hpp>

static gl::Program::Uniform UNIFORM_TR("tr");
static gl::Program::Uniform UNIFORM_SQ("sq");
static gl::Program::Uniform UNIFORM_RATIO("ratio");
static gl::Program::Uniform UNIFORM_ITERATIONS("iterations");

FractalView::FractalView() : mTransform(1.f) {
    setExpanding();
    mShader.load(R"(
#version 330 core

in vec4 pos;
in vec2 uv;
out vec2 pass_uv;
uniform float ratio;
uniform mat4 SL_uniform_transform;

void main() {
    gl_Position = SL_uniform_transform * pos;
    pass_uv = (uv - 0.5) * 2.0;
    pass_uv.x *= ratio;
}
)",
    R"(
#version 330 core

in vec2 pass_uv;

#define product(a, b) vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)
#define conjugate(a) vec2(a.x,-a.y)
#define divide(a, b) vec2(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))

uniform float c;
uniform int iterations;
uniform mat4 tr;
uniform sampler2D tex;

out vec4 color;

void main() {
    vec2 tmp_pass_uv = (tr * vec4(pass_uv, 0.0, 1.0)).xy;
    vec2 v = vec2(0, 0);
    int i = 0;
    for (; i < iterations; ++i) {
        v = product(v, v) + tmp_pass_uv;
        vec2 tmp = v * v;
        if ((v.x + v.y) > 4) {
            break;
        }
    }
    if (i == iterations) {
        color = vec4(0, 0, 0, 1);
    } else {
        float theta = float(i) / float(iterations);
        color = texture(tex, vec2(theta, 0));
    }
}
)", { "pos", "uv" }, gl::GLSLOptions { .custom = true });
    mShader.compile();
    mShader.use();
    mShader.set(UNIFORM_TR, mTransform);
    mShader.set(UNIFORM_SQ, 1.f);

    mTexture = _new<gl::Texture2D>();
    mTexture->tex2D(*AImage::fromUrl(":img/color_scheme_wikipedia.png"));
}

void FractalView::render(ARenderContext context) {
    AView::render(context);

    mShader.use();
    mTexture->bind();
    context.render.rectangle(ACustomShaderBrush {}, { 0, 0 }, getSize());
}

void FractalView::setSize(glm::ivec2 size) {
    AView::setSize(size);
    mShader.use();
    mShader.set(UNIFORM_RATIO, mAspectRatio = float(size.x) / float(size.y));
}

void FractalView::setIterations(unsigned it) {
    mShader.use();
    mShader.set(UNIFORM_ITERATIONS, int(it));
}

void FractalView::onScroll(const AScrollEvent& event) {
    AView::onScroll(event);
    auto projectedPos = (glm::dvec2(event.origin) / glm::dvec2(getSize()) - glm::dvec2(0.5)) * 2.0;
    projectedPos.x *= mAspectRatio;
    mTransform = glm::translate(mTransform, glm::vec3 { projectedPos, 0.0 });
    mTransform = glm::scale(mTransform, glm::vec3(1.0 - event.delta.y / 1000.0));
    mTransform = glm::translate(mTransform, -glm::vec3 { projectedPos, 0.0 });

    handleMatrixUpdated();

    redraw();
}

void FractalView::reset() {
    mTransform = glm::dmat4(1.0);
    handleMatrixUpdated();
    redraw();
}

void FractalView::handleMatrixUpdated() {
    mShader.use();
    mShader.set(UNIFORM_TR, mTransform);
    emit centerPosChanged(getPlotPosition(), getPlotScale());
}

void FractalView::onKeyDown(AInput::Key key) {
    AView::onKeyDown(key);
    onKeyRepeat(key);
}

void FractalView::onKeyRepeat(AInput::Key key) {
    AView::onKeyRepeat(key);
    constexpr float SPEED = 0.2f;
    switch (key) {
        case AInput::UP:
            mTransform = glm::translate(mTransform, { 0, -SPEED, 0 });
            break;
        case AInput::DOWN:
            mTransform = glm::translate(mTransform, { 0, SPEED, 0 });
            break;
        case AInput::LEFT:
            mTransform = glm::translate(mTransform, { -SPEED, 0, 0 });
            break;
        case AInput::RIGHT:
            mTransform = glm::translate(mTransform, { SPEED, 0, 0 });
            break;
        case AInput::PAGEDOWN:
            mTransform = glm::scale(mTransform, glm::vec3 { 0.99 });
            break;
        case AInput::PAGEUP:
            mTransform = glm::scale(mTransform, glm::vec3 { 1.01 });
            break;

        default:
            return;
    }
    handleMatrixUpdated();
}

glm::dvec2 FractalView::getPlotPosition() const { return glm::dvec2(mTransform[3]); }

double FractalView::getPlotScale() const { return mTransform[0][0]; }

void FractalView::setPlotPositionAndScale(glm::dvec2 position, double scale) {
    mTransform = glm::dmat4(scale);
    mTransform[3] = glm::dvec4 { position, 0, 1 };
    handleMatrixUpdated();
}
