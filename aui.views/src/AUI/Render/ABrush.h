// AUI Framework - Declarative UI toolkit for modern C++17
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
// Created by Alex2772 on 11/18/2021.
//

#pragma once


#include <variant>
#include <AUI/Common/AColor.h>
#include <AUI/Enum/Repeat.h>
#include "AUI/Enum/ImageRendering.h"


/**
 * The simplest brush which used single solid color to fill.
 */
struct ASolidBrush {
    AColor solidColor = AColor::WHITE;
};


/**
 * The brush used with a custom graphics API shaders.
 */
struct ACustomShaderBrush {

};


/**
 * Brush which produces linear gradient by rectangle.
 */
struct ALinearGradientBrush {
    AColor topLeftColor = 1.f;
    AColor topRightColor = 1.f;
    AColor bottomLeftColor = 1.f;
    AColor bottomRightColor = 1.f;
};

class ITexture;

/**
 * Brush which draws texture (raster image).
 */
struct ATexturedBrush {
    _<ITexture> texture;

    /**
     * Optional. Top left UV coords of the texture.
     * <dl>
     *     <dt><b>Warning!</b></dt>
     *     <dd>When set, on software renderer it produces extra CPU costs.</dd>
     * </dl>
     */
    AOptional<glm::vec2> uv1;

    /**
     * Optional. Top left UV coords of the texture.
     * <dl>
     *     <dt><b>Warning!</b></dt>
     *     <dd>When set, on software renderer it produces extra CPU costs.</dd>
     * </dl>
     */
    AOptional<glm::vec2> uv2;

    /**
     * Optional. Controls how does the image is scaled.
     * <dl>
     *     <dt><b>Warning!</b></dt>
     *     <dd>When set to <code>ImageRendering::SMOOTH</code>, on software renderer it produces extra CPU costs.</dd>
     * </dl>
     */
    ImageRendering imageRendering = ImageRendering::PIXELATED;

    /**
     * Optional. Controls how does the image behaves when it's size is not enough to cover the whole rect.
     */
    Repeat repeat = Repeat::NONE;
};


/**
 * Device-independent brush (shader) used for drawing.
 */
using ABrush = std::variant<ASolidBrush,
                            ALinearGradientBrush,
                            ATexturedBrush,
                            ACustomShaderBrush>;