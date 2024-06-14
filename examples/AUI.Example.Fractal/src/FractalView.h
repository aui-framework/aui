/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

