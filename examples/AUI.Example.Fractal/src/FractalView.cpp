/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2772 on 12/9/20.
//

#include "FractalView.h"
#include <AUI/Render/Render.h>
#include <glm/gtc/matrix_transform.hpp>

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
    mShader.set("tr", mTransform);
    mShader.set("sq", 1.f);

    mTexture = _new<gl::Texture2D>();
    mTexture->tex2D(AImage::fromUrl(":img/color_scheme_wikipedia.png"));
}

void FractalView::render() {
    AView::render();

    mShader.use();
    mTexture->bind();
    Render::drawRect(0, 0, getWidth(), getHeight());
}

void FractalView::setSize(glm::ivec2 size) {
    AView::setSize(size);
    mShader.use();
    mShader.set("ratio", mAspectRatio = float(width) / float(height));
}

void FractalView::setIterations(unsigned it) {
    mShader.use();
    mShader.set("iterations", int(it));
}

void FractalView::onMouseWheel(glm::ivec2 pos, int delta) {
    AView::onMouseWheel(pos, delta);
    auto projectedPos = (glm::dvec2(pos) / glm::dvec2(getSize()) - glm::dvec2(0.5)) * 2.0;
    projectedPos.x *= mAspectRatio;
    mTransform = glm::translate(mTransform, glm::dvec3{projectedPos, 0.0});
    mTransform = glm::scale(mTransform, glm::dvec3(1.0 - delta / 1000.0));
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
    mShader.set("tr", mTransform);
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
        case AInput::PageDown:
            mTransform = glm::scale(mTransform, glm::dvec3{0.99});
            break;
        case AInput::PageUp:
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
