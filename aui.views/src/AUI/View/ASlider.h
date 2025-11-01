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

#include "AViewContainer.h"
#include "AProgressBar.h"

namespace declarative {

/**
 * @brief Slider control.
 * ![](imgs/views/ASlider.png)
 * @details
 *
 * The `Slider` represents a horizontal slider control that can be
 * used in declarative UI definitions. It exposes a single value in the
 * range `[0, 1]` and emits a signal whenever the value changes. The visual
 * representation consists of a *track* and a *handle* view. By default the
 * track and handle are created using the static helper functions
 * `defaultTrack()` and `defaultHandle()`, but users may provide custom
 * implementations by overriding the corresponding member variables.
 *
 * The slider automatically handles mouse and keyboard input. When the user
 * drags the handle, the `onValueChanged` slot is invoked.
 *
 * @ingroup views_input
 */
struct API_AUI_VIEWS Slider {
    /**
     * @brief Current slider value in the range `[0, 1]`.
     * @details
     * The value is a contract input, meaning it can be bound to other
     * components or data models. Changing this value programmatically will
     * update the handle position.
     */
    contract::In<aui::float_within_0_1> value;

    /**
     * @brief Slot that is called whenever the slider value changes.
     * @details
     * The slot receives the new value as a `float_within_0_1`. The
     * implementation expects the consumer to update the `value`
     * contract inside this slot.
     */
    contract::Slot<aui::float_within_0_1> onValueChanged;

    /**
     * @brief View representing the slider track.
     * @details
     * By default this is created by `defaultTrack(value)` which creates a
     * simple rectangular track. Users can replace this with a custom view
     * to change the appearance.
     */
    _<AView> track = defaultTrack(value);

    /**
     * @brief View representing the slider handle.
     * @details
     * The default implementation is provided by `defaultHandle()`.
     */
    _<AView> handle = defaultHandle();

    /**
     * @brief Creates a default track view.
     * @param value The slider value contract.
     * @return A view representing the track.
     * @details
     * The returned view is a simple rectangle that visually represents
     * the slider's range. It listens to the `value` contract to update its
     * visual state.
     *
     * <!-- aui:snippet aui.views/src/AUI/View/ASlider.cpp defaultTrack -->
     *
     */
    static _<AView> defaultTrack(const contract::In<aui::float_within_0_1>& value);

    /**
     * @brief Creates a default handle view.
     * @return A view representing the handle.
     * @details
     * The handle is a draggable element that the user can move along the
     * track. It updates the `value` contract when dragged.
     *
     * <!-- aui:snippet aui.views/src/AUI/View/ASlider.cpp defaultHandle -->
     */
    static _<AView> defaultHandle();

    _<AView> operator()();
};
}   // namespace declarative
