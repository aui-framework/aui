// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#pragma once


#include <AUI/View/AView.h>
#include <AUI/GL/Program.h>
#include <AUI/GL/Texture2D.h>

class FractalView: public AView {
private:
    gl::Program mShader;
    _<gl::Texture2D> mTexture;
    glm::dmat4 mTransform;

    float mAspectRatio;

    void handleMatrixUpdated();

public:
    FractalView();
    void render(ClipOptimizationContext context) override;

    void reset();

    void setIterations(unsigned it);

    void onKeyDown(AInput::Key key) override;

    void onKeyRepeat(AInput::Key key) override;

    void onScroll(glm::ivec2 pos, glm::ivec2 delta) override;

    void setSize(glm::ivec2 size) override;

    gl::Shader& getShader() {
        return mShader;
    }

    const _<gl::Texture2D>& getTexture() const {
        return mTexture;
    }

    glm::dvec2 getPlotPosition() const;
    double getPlotScale() const;

    void setPlotPositionAndScale(glm::dvec2 position, double scale);

signals:

    emits<glm::dvec2, double> centerPosChanged;
};

