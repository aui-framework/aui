// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2772 on 12/9/20.
//

#include "FractalView.h"
#include "AUI/Render/ARender.h"
#include <glm/gtc/matrix_transform.hpp>

static gl::Shader::Uniform UNIFORM_TR("tr");
static gl::Shader::Uniform UNIFORM_SQ("sq");
static gl::Shader::Uniform UNIFORM_RATIO("ratio");
static gl::Shader::Uniform UNIFORM_ITERATIONS("iterations");

FractalView::FractalView():
    mTransform(1.f)
{
    setExpanding();
    mShader.load(
            "in vec4 pos;"
                 "in vec2 uv;"
                 "out vec2 pass_uv;"
                 "uniform float ratio;"

                 "void main() {"
                    "gl_Position = pos;"
                    "pass_uv = (uv - 0.5) * 2.0;"
                    "pass_uv.x *= ratio;"
                 "}",
                 "in vec2 pass_uv;\n"

                 "#define product(a, b) dvec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)\n"
                 "#define conjugate(a) dvec2(a.x,-a.y)\n"
                 "#define divide(a, b) dvec2(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))\n"
                 "uniform float c;"
                 "uniform int iterations;"
                 "uniform dmat4 tr;"
                 "uniform sampler2D tex;"

                 "out vec4 color;"
                 "void main() {"
                    "dvec2 tmp_pass_uv = (tr * dvec4(pass_uv, 0.0, 1.0)).xy;"
                    "dvec2 v = dvec2(0, 0);"
                    "int i = 0;"
                    "for (; i < iterations; ++i) {"
                        "v = product(v, v) + tmp_pass_uv;"
                        "dvec2 tmp = v * v;"
                        "if ((v.x + v.y) > 4) {"
                            "break;"
                        "}"
                    "}"
                    "if (i == iterations) {"
                        "color = vec4(0, 0, 0, 1);"
                    "} else {"
                        "float theta = float(i) / float(iterations);"
                        "color = texture(tex, vec2(theta, 0));"
                    "}"
                 "}", {"pos", "uv"}, "400");
    mShader.compile();
    mShader.use();
    mShader.set(UNIFORM_TR, mTransform);
    mShader.set(UNIFORM_SQ, 1.f);

    mTexture = _new<gl::Texture2D>();
    mTexture->tex2D(*AImage::fromUrl(":img/color_scheme_wikipedia.png"));
}

void FractalView::render(ClipOptimizationContext context) {
    AView::render(context);

    mShader.use();
    mTexture->bind();
    ARender::rect(ACustomShaderBrush{}, {0, 0}, getSize());
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

void FractalView::onScroll(glm::ivec2 pos, glm::ivec2 delta) {
    AView::onScroll(pos, delta);
    auto projectedPos = (glm::dvec2(pos) / glm::dvec2(getSize()) - glm::dvec2(0.5)) * 2.0;
    projectedPos.x *= mAspectRatio;
    mTransform = glm::translate(mTransform, glm::dvec3{projectedPos, 0.0});
    mTransform = glm::scale(mTransform, glm::dvec3(1.0 - delta.y / 1000.0));
    mTransform = glm::translate(mTransform, -glm::dvec3{projectedPos, 0.0});

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
            mTransform = glm::translate(mTransform, {0, -SPEED, 0});
            break;
        case AInput::DOWN:
            mTransform = glm::translate(mTransform, {0, SPEED, 0});
            break;
        case AInput::LEFT:
            mTransform = glm::translate(mTransform, {-SPEED, 0, 0});
            break;
        case AInput::RIGHT:
            mTransform = glm::translate(mTransform, {SPEED, 0, 0});
            break;
        case AInput::PAGEDOWN:
            mTransform = glm::scale(mTransform, glm::dvec3{0.99});
            break;
        case AInput::PAGEUP:
            mTransform = glm::scale(mTransform, glm::dvec3{1.01});
            break;

        default:
            return;
    }
    handleMatrixUpdated();
}

glm::dvec2 FractalView::getPlotPosition() const {
    return glm::dvec2(mTransform[3]);
}

double FractalView::getPlotScale() const {
    return mTransform[0][0];
}

void FractalView::setPlotPositionAndScale(glm::dvec2 position, double scale) {
    mTransform = glm::dmat4(scale);
    mTransform[3] = glm::dvec4{position, 0, 1};
    handleMatrixUpdated();
}
