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
#include "AUI/Render/ACanvas.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Render/ARender/GL/OpenGLBackend.hpp>

static gl::Program::Uniform UNIFORM_TR("tr");
static gl::Program::Uniform UNIFORM_SQ("sq");
static gl::Program::Uniform UNIFORM_RATIO("ratio");
static gl::Program::Uniform UNIFORM_ITERATIONS("iterations");
static gl::Program::Uniform UNIFORM_TRANSFORM("SL_uniform_transform");
static gl::Program::Uniform UNIFORM_TEX("tex");

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

#define product(a) vec2(a.x*a.x-a.y*a.y, 2.0*a.x*a.y)

uniform int iterations;
uniform mat4 tr;
uniform sampler2D tex;

out vec4 color;

void main() {
    vec2 tmp_pass_uv = (tr * vec4(pass_uv, 0.0, 1.0)).xy;
    vec2 v = vec2(0, 0);
    int i = 0;
    for (; i < iterations; ++i) {
        v = product(v) + tmp_pass_uv;
        if (dot(v, v) > 4.0) {
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
    mShader.set(UNIFORM_TEX, 0);

    mTexture = _new<gl::Texture2D>();
    mTexture->tex2D(*AImage::fromUrl(":img/color_scheme_wikipedia.png"));
    mTexture->setupLinear();
    mTexture->setupClampToEdge();
}

void FractalView::render(ARenderContext context) {
    AView::render(context);

    if (getSize().x <= 0 || getSize().y <= 0) return;

    if (mFboTexture.getSize() != glm::u32vec2(getSize())) {
        mFboTexture.tex2D(getSize(), APixelFormat::R8G8B8A8_UNORM);
        mFbo.bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFboTexture.getHandle(), 0);

        if (auto backend = dynamic_cast<OpenGLBackend*>(&context.backend)) {
            mWrappedFboTexture = backend->createFramebufferWrapper(mFboTexture.getHandle(), getSize());
            mWrappedFboTexture->setOrigin(TextureOrigin::BOTTOM_LEFT);
        }
    }

    if (mWrappedFboTexture) {
        mFbo.bind();
        glViewport(0, 0, getSize().x, getSize().y);
        mShader.use();
        mShader.set(UNIFORM_TR, mTransform);
        mShader.set(UNIFORM_RATIO, mAspectRatio);
        mShader.set(UNIFORM_ITERATIONS, (int)mIterations);
        mShader.set(UNIFORM_TRANSFORM, glm::mat4(1.f));
        mTexture->bind(0);

        static constexpr glm::vec2 pos[] = {
            {-1, -1}, { 1, -1}, {-1,  1}, { 1,  1}
        };
        static constexpr glm::vec2 uv[] = {
            {0, 0}, {1, 0}, {0, 1}, {1, 1}
        };
        mQuadVao.insert(0, AArrayView<glm::vec2>(pos, 4), "pos");
        mQuadVao.insert(1, AArrayView<glm::vec2>(uv, 4), "uv");
        mQuadVao.bind();
        mQuadVao.drawArrays(GL_TRIANGLE_STRIP, 4);

        gl::Framebuffer::unbind();

        context.canvas.rectangle(APaint { ATexturedBrush { mWrappedFboTexture } }, { 0, 0 }, getSize());
    }
}

void FractalView::setSize(glm::ivec2 size) {
    AView::setSize(size);
    mAspectRatio = float(size.x) / float(size.y);
}

void FractalView::setIterations(unsigned it) {
    mIterations = it;
    redraw();
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
