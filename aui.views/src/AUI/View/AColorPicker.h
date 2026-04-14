// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <AUI/View/AGroupBox.h>
#include <AUI/View/ASlider.h>

namespace declarative::color_picker {

/**
 * @brief View that displays a single color.
 * @ingroup views_indication
 * @details
 * The `ColorView` simply renders a solid rectangle matching the provided color contract. It is intended to be used as a
 * visual preview of a color value.
 *
 * <!-- aui:snippet examples/ui/color_picker1/src/main.cpp colorpicker_example -->
 */
struct ColorView {
    /**
     * @brief The color to display.
     */
    contract::In<AColor> color;

    _<AView> operator()() {
        auto color = this->color;
        return Centered {
            _new<AView>() AUI_LET {
                    AObject::connect(
                        AUI_REACT(ass::PropertyListRecursive {
                          BackgroundSolid {
                            color.value(),
                          },
                          Expanding {},
                        }),
                        AUI_SLOT(it)::setCustomStyle);
                },
        };
    }
};

/**
 * @brief RGB sliders for editing a color.
 * @ingroup views_input
 * @details
 * `SlidersRGB` exposes three sliders, one for each channel (red, green,
 * blue). The sliders are bound to a shared `AColor` contract and notify a
 * callback when the color changes.
 *
 * <!-- aui:snippet examples/ui/color_picker1/src/main.cpp colorpicker_example -->
 */
struct SlidersRGB {
    /**
     * @brief Pointer to a color channel.
     */
    using Channel = float(AColor::*);

    /**
     * @brief The color contract to edit.
     */
    contract::In<AColor> color;

    /**
     * @brief Callback invoked when the color changes.
     */
    std::function<void(AColor)> onColorChanged;

    /**
     * @brief Factory used to create individual sliders.
     */
    std::function<_<AView>(contract::In<AColor> color, std::function<void(AColor)> onColorChanged, Channel channel)>
        sliderFactory = defaultSlider;

    /**
     * @brief Creates a slider for a single channel.
     * @param color The color contract.
     * @param onColorChanged Callback when the color changes.
     * @param channel Pointer to the channel member.
     * @return A view containing the slider.
     */
    static _<AView>
    defaultSlider(contract::In<AColor> color, std::function<void(AColor)> onColorChanged, Channel channel) {
        return Slider {
            .value = AUI_REACT(std::invoke(channel, color)),
            .onValueChanged =
                [color, onColorChanged = std::move(onColorChanged), channel](aui::float_within_0_1 v) {
                    auto copy = *color;
                    std::invoke(channel, copy) = v;
                    onColorChanged(copy);
                },
            .track =
                _new<AView>() AUI_LET {
                    AObject::connect(
                        AUI_REACT(ass::PropertyListRecursive {
                          Expanding {},
                          BackgroundGradient {
                            adjustChannel(color.value(), channel, 0.0f),
                            adjustChannel(color.value(), channel, 1.0f),
                            90_deg,
                          },
                          MinSize { 20_dp },
                        }),
                        AUI_SLOT(it)::setCustomStyle);
                },
            .handle =
                _new<AView>() AUI_OVERRIDE_STYLE {
                  FixedSize { 3_px, {} },
                  Expanding {},
                  BackgroundSolid { AColor::WHITE },
                  Border { 1_px, AColor::BLACK },
                  Margin { 4_px, 0 },
                },
        };
    }

    _<AView> operator()() {
        return Vertical {
            sliderFactory(color, onColorChanged, &AColor::r),
            sliderFactory(color, onColorChanged, &AColor::g),
            sliderFactory(color, onColorChanged, &AColor::b),
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } };
    }

private:
    /**
     * @brief Adjusts a single channel of a color.
     * @param src The source color.
     * @param channel Pointer to the channel member.
     * @param newValue The new channel value.
     * @return The color with the channel updated.
     */
    static AColor adjustChannel(AColor src, float(AColor::* channel), float newValue) {
        std::invoke(channel, src) = newValue;
        return src;
    }
};
}   // namespace declarative::color_picker
