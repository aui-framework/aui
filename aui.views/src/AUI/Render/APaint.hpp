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

#pragma once

#include <AUI/Common/AColor.h>
#include <AUI/Render/ABrush.h>

/**
 * @brief Blending mode.
 * @details
 * <p><b>Terminology used in this documentation</b>:</p>
 * <dl>
 *   <dt><b><u>S</u>ource color</b> (S)</dt>
 *   <dd>
 *      Source color is a color of the brush (i.e. texture color matching current position) multiplied by the current
 *      renderer color, i.e. when drawing a black rectangle onto the white canvas the source color is black.
 *   </dd>
 *
 *   <dt><b><u>D</u>estination color</b> (D)</dt>
 *   <dd>
 *      Destination color is a color of the framebuffer you're drawing to, i.e. when drawing a black
 *      rectangle onto the white canvas the destination color is white.
 *   </dd>
 *
 *   <dt><b><u>S</u>.rgb</b></dt>
 *   <dd>Source color without the alpha component.</dd>
 *
 *   <dt><b><u>S</u>.a</b></dt>
 *   <dd>Source's alpha component without the color itself.</dd>
 *
 *   <dt><b><u>D</u>.rgb</b></dt>
 *   <dd>Destination color without the alpha component.</dd>
 *
 *   <dt><b><u>D</u>.a</b></dt>
 *   <dd>Destination's alpha component without the color itself.</dd>
 *
 *   <dt><b>Alpha-based</b></dt>
 *   <dd>Alpha-based blending mode is a blending mode that uses the alpha component in it's formula.</dd>
 *
 *   <dt><b>Color-based</b></dt>
 *   <dd>Color-based blending mode is a blending mode that does not use the alpha component in it's formula.</dd>
 * </dl>
 * <!-- aui:no_dedicated_page -->
 */
enum class Blending {
    /**
     * @brief Normal blending.
     * @details
     * <dl>
     *   <dt><b>Formula</b></dt>
     *   <dd><code>S.rgb + D.rgb * (1 - S.a)</code> (assuming source is premultiplied)</dd>
     *   <dt><b>Type</b></dt>
     *   <dd>Alpha-based</dd>
     *   <dt><b>Behaviour</b></dt>
     *   <dd>
     *     <p>When <code>S.a</code> is 0, <code>NORMAL</code> does not drawElements anything.
     *     <p>When <code>S.a</code> is 1, <code>NORMAL</code> ignores <code>D</code>.
     *   </dd>
     * </dl>
     */
    NORMAL,


    /**
     * @brief Simply sums <code>S</code> and <code>D</code> colors.
     * @details
     * <dl>
     *   <dt><b>Formula</b></dt>
     *   <dd><code>S.rgb + D.rgb</code></dd>
     *   <dt><b>Type</b></dt>
     *   <dd>Color-based</dd>
     *   <dt><b>Behaviour</b></dt>
     *   <dd>
     *     <p>When <code>S</code> is black, <code>ADDITIVE</code> does not drawElements anything.</p>
     *     <p>When <code>S</code> is white, <code>ADDITIVE</code> draws white.</p>
     *   </dd>
     * </dl>
     */
    ADDITIVE,

    /**
     * @brief Inverses destination color and multiplies it with the source color.
     * @details
     * <dl>
     *   <dt><b>Formula</b></dt>
     *   <dd><code>S.rgb * (1 - D.rgb)</code></dd>
     *   <dt><b>Type</b></dt>
     *   <dd>Color-based</dd>
     *   <dt><b>Behaviour</b></dt>
     *   <dd>
     *     <p>When <code>S</code> is black, <code>INVERSE_DST</code> does not drawElements anything.</p>
     *     <p>When <code>S</code> is white, <code>INVERSE_DST</code> does full inverse.</p>
     *   </dd>
     * </dl>
     */
    INVERSE_DST,

    /**
     * @brief Inverses source color and multiplies it with the destination color.
     * @details
     * <dl>
     *   <dt><b>Formula</b></dt>
     *   <dd><code>(1 - S.rgb) * D.rgb</code></dd>
     *   <dt><b>Type</b></dt>
     *   <dd>Color-based</dd>
     *   <dt><b>Behaviour</b></dt>
     *   <dd>
     *     <p>When <code>S</code> is black, <code>INVERSE_SRC</code> does not drawElements anything.</p>
     *     <p>When <code>S</code> is white, <code>INVERSE_SRC</code> draws black.</p>
     *   </dd>
     * </dl>
     */
    INVERSE_SRC,
};

struct APaint {
    ABrush brush = ASolidBrush{AColor::WHITE};
    AColor color = AColor::WHITE;
    Blending blending = Blending::NORMAL;
    float opacity = 1.0f;

    bool operator==(const APaint&) const = default;
};
