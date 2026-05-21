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

/**
 * @brief Blending mode.
 * @ingroup views
 */
enum class Blending {
    /**
     * @brief Normal alpha blending.
     * @details
     * <dl>
     *   <dt><b>Formula</b></dt>
     *   <dd><code>S.rgb * S.a + D.rgb * (1 - S.a)</code></dd>
     *   <dt><b>Type</b></dt>
     *   <dd>Alpha-based</dd>
     * </dl>
     */
    NORMAL,

    /**
     * @brief Additive blending.
     * @details
     * <dl>
     *   <dt><b>Formula</b></dt>
     *   <dd><code>S.rgb + D.rgb</code></dd>
     *   <dt><b>Type</b></dt>
     *   <dd>Color-based</dd>
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
     *     <p>When <code>D</code> is black, <code>INVERSE_DST</code> draws <code>S</code>.</p>
     *     <p>When <code>D</code> is white, <code>INVERSE_DST</code> does not drawElements anything.</p>
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
