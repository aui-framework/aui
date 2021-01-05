/**
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once


#include <AUI/View/AView.h>
#include <AUI/GL/Shader.h>
#include <AUI/GL/Texture2D.h>

class FractalView: public AView {
private:
    GL::Shader mShader;
    _<GL::Texture2D> mTexture;
    glm::dmat4 mTransform;

    float mAspectRatio;

    void handleMatrixUpdated();

public:
    FractalView();
    void render() override;

    void reset();

    void setIterations(unsigned it);

    void onKeyDown(AInput::Key key) override;

    void onKeyRepeat(AInput::Key key) override;

    void onMouseWheel(glm::ivec2 pos, int delta) override;

    void setSize(int width, int height) override;

    GL::Shader& getShader() {
        return mShader;
    }

    const _<GL::Texture2D>& getTexture() const {
        return mTexture;
    }

    glm::dvec2 getPlotPosition() const;
    double getPlotScale() const;

    void setPlotPositionAndScale(glm::dvec2 position, double scale);

signals:

    emits<glm::dvec2, double> centerPosChanged;
};

