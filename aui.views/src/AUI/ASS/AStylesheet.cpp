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

//
// Created by alex2 on 29.12.2020.
//

#include "AUI/Enum/AOverflow.h"
#include "AUI/View/AScrollAreaViewport.h"
#if AUI_PLATFORM_WIN
#include <dwmapi.h>
#endif

#include <AUI/View/AButton.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/View/AAbstractTextField.h>
#include <AUI/View/ANumberPicker.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/ADropdownList.h>
#include <AUI/View/AListView.h>
#include <AUI/View/AHDividerView.h>
#include <AUI/View/AVDividerView.h>
#include <AUI/View/ATabView.h>
#include <AUI/Platform/ACustomCaptionWindow.h>
#include <AUI/View/ARulerView.h>
#include <AUI/View/ATreeView.h>
#include <AUI/View/AText.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AProgressBar.h>
#include <AUI/View/ACircleProgressBar.h>
#include "AStylesheet.h"
#include "ASS.h"
#include "AUI/View/ASpinner.h"
#include "AUI/View/AGroupBox.h"
#include "AUI/View/ASlider.h"
#include "AUI/View/ATextArea.h"
#include "AUI/View/ASpinnerV2.h"
#include "AUI/View/ATextField.h"

AStylesheet::AStylesheet() {
    using namespace ass;
    using namespace std::chrono_literals;

    static auto getOsThemeColorLighter = [&]() -> AColor {
        return glm::mix(glm::vec4(getOsThemeColor()), glm::vec4(AColor::WHITE), 0.17f);
    };

    addRules({
        // COMMON VIEWS ======================================
        // AView
        {
            t<AView>(),
            TextColor { 0x0_rgb },
            FontSize {10_pt},
            FontRendering::SUBPIXEL,
            ImageRendering::SMOOTH,
            MaxSize { 9999999_dp, 9999999_dp },
        },

        // AWindow
        {
            t<AWindow>(),
            BackgroundSolid {0xf0f0f0_rgb},
#if AUI_PLATFORM_IOS
            Padding { 24_dp, 4_dp, 4_dp }, // TODO temporary fix
#else
            Padding { 4_dp },
#endif
        },

         // ASpinner
        {
            t<ASpinner>(),
            BackgroundImage { ":uni/svg/spinner.svg", 0x404040_rgb },
            FixedSize { 16_dp }
        },

        /// [ASpinnerV2]
        {
            t<ASpinnerV2>(),
            BackgroundImage { ":uni/svg/spinner_v2.svg", 0x993c3c43_argb },
            FixedSize { 16_dp },
            ASpinnerV2::Configuration { .period = 1000ms, .steps = 8 },
        },
        /// [ASpinnerV2]

        // AText
        {
            { t<AText>(), t<ATextArea>() },
            LineHeight { 1.f },
            VerticalAlign::MIDDLE,
            Expanding { 1, 0 },
            LineHeight::NORMAL,
        },

        {
            t<ATextArea>(),
            TextColor { inherit },
            VerticalAlign::DEFAULT,
            Expanding { 1 },
        },
        // AAbstractLabel
        {
            t<AAbstractLabel>(),
            VerticalAlign::MIDDLE,
            TextColor { inherit },
        },
        {
            t<AAbstractLabel>::disabled(),
            TextColor { 0x444444_rgb },
        },

        /// [AButton]
        {
            t<AButton>(),
            BackgroundSolid {0xffffff_rgb},
            Padding {3_dp, 5_dp},
            MinSize {60_dp, 22_dp },
            Border { 1_dp, 0xcacaca_rgb },
            BorderRadius {5_dp},
            ATextAlign::CENTER,
            VerticalAlign::MIDDLE,
            BoxShadow {{}, 1_dp, 4_dp, -2_dp, 0x80000000_argb},
        },
        {
            t<AButton>::hover(),
            Border {1_dp, getOsThemeColor() * glm::vec4(1, 1, 1, 0.3f)}
        },
        {
            t<AButton>::active(),
            BackgroundSolid{0xfafafa_rgb},
        },
        {
            button::Default(t<AButton>()),
            FontRendering::ANTIALIASING,
            BackgroundGradient { ALinearGradientBrush{
                    .colors = {
                            {0.f, getOsThemeColorLighter()},
                            {0.f, getOsThemeColor()},
                    },
            } },
            BoxShadow { 0, 1_dp, 3_dp, -1_dp, getOsThemeColor() },
            Border { nullptr },
            TextColor { 0xffffff_rgb },
        },
        {
            button::Default(t<AButton>::hover()),
            BoxShadow { 0, 1_dp, 6_dp, -1_dp, getOsThemeColor() },
        },
        {
            t<AButton>::active(),
            BoxShadow { nullptr },
        },
        {
            button::Default(t<AButton>::hover()),
            BackgroundGradient { ALinearGradientBrush{
                    .colors = {
                            {0.f, getOsThemeColorLighter()},
                            {1.f, getOsThemeColor()},
                    },
            } },
        },
        {
            button::Default(t<AButton>::active()),
            BackgroundSolid { getOsThemeColor() }
        },
        {
            t<AButton>::disabled(),
            BackgroundSolid { 0xcccccc_rgb },
            BoxShadow { nullptr },
            Border { 1_dp, 0xbfbfbf_rgb },
            TextColor { 0x838383_rgb }
        },
        /// [AButton]

        // Text fields
        {
            t<AAbstractTypeable>(),
            ACursor::TEXT,
        },
        {
            { t<ATextField>(), c(".number-picker") >> t<AAbstractTypeable>() },
            Padding { 3_dp, 6_dp },
        },
        {
            class_of(".input-field"),
            Padding { 3_dp, 6_dp },
            TextColor { 0_rgb },
            BackgroundSolid { 0xffffff_rgb },
            Border { 1_dp, 0xa0a0a0_rgb },
            BorderRadius { 4_dp },
            MinSize { 100_dp, 22_dp },
            AOverflow::HIDDEN,
        },
        {
            class_of::hover(".input-field"),
            Border { 1_dp, 0x404040_rgb },
        },
        {
            class_of::focus(".input-field"),
            Border { 1_dp, getOsThemeColor() },
        },
        {
            class_of::disabled(".input-field"),
            Border { 1_dp, 0x999999_rgb },
            TextColor { 0x555555_rgb },
            ACursor::DEFAULT,
            BackgroundSolid { 0xe0e0e0_rgb }
        },

        // ANumberPicker
        {
            c(".number-picker"),
            MinSize { 60_dp, 22_dp },
            Padding { 1_px },
            AOverflow::HIDDEN,
        },
        {
            class_of(".up-down-wrapper"),
            Margin { 0 },
            LayoutSpacing { 1_px },
            //BackgroundSolid { 0xf0f0f0_rgb }
        },
        {
            c(".number-picker") >> t<AButton>(),
            Padding { 3_dp, 0 },
            FixedSize { 19_dp, 10_dp },
            MinSize { {}, 3_dp },
            BoxShadow { nullptr },
            BorderRadius { 0 },
            Margin { 0 },
            Border { nullptr },
            BackgroundImage {{}, getOsThemeColor().readableBlackOrWhite().opacify(0.7f), {}, Sizing::FIT_PADDING },
        },
        {
            c(".number-picker") >> c(".up"),
            BackgroundImage { ":uni/svg/up.svg", {}, {}, Sizing::CENTER },
        },
        {
            c(".number-picker") >> c(".down"),
            BackgroundImage { ":uni/svg/down.svg", {}, {}, Sizing::CENTER },
        },

        /// [ACheckBox]
        {
            t<ACheckBox>(), // styles the checkbox + contents container
            LayoutSpacing { 4_dp }, // specifies space between ACheckBox::Box and contents
        },
        {
            t<ACheckBox::Box>(), // styling box itself
            BackgroundSolid { 0xffffff_rgb },
            Border { 1_dp, 0x333333_rgb },
            FixedSize { 14_dp, 14_dp },
            BackgroundImage { {}, 0x333333_rgb },
            BorderRadius { 3_dp },
        },
        {
            t<ACheckBox>::active() >> t<ACheckBox::Box>(),
            BackgroundSolid { AColor::GRAY.transparentize(0.8f) },
        },
        {
            Selected(t<ACheckBox::Box>()),
            BackgroundImage { ":uni/svg/checkbox.svg", getOsThemeColor().readableBlackOrWhite() },
            Border { nullptr },
            BackgroundGradient { getOsThemeColorLighter(), getOsThemeColor(), 180_deg },
        },
        {
            t<ACheckBox>::active() > Selected(t<ACheckBox::Box>()),
            BackgroundSolid { AColor::GRAY.transparentize(0.8f) },
            BackgroundGradient { getOsThemeColorLighter().lighter(0.3f), getOsThemeColor(), 180_deg },
        },
        {
            t<ACheckBox::Box>::disabled(),
            BackgroundSolid { 0xe5e5e5_rgb },
            Border { 1_px, 0xa0a0a0_rgb },
        },
        {
            Selected(t<ACheckBox::Box>::disabled()),
            BackgroundGradient { AColor::GRAY.lighter(0.1f), AColor::GRAY.darker(0.1f), 0_deg },
        },
        /// [ACheckBox]

        // ARulerView
        {
            t<ARulerView>(),
                MinSize { 16_dp },
                BackgroundSolid { 0x20000000_argb },
            TextColor { 0x40000000_argb },
                AOverflow::HIDDEN,
                FontSize { 8_pt },
        },
        {
            c(".arulerarea-content"),
            Border { 1_px, 0x40808080_argb },
        },
        {
            c(".arulerarea-unit"),
            MaxSize { 16_dp },
            BackgroundSolid { 0x20000000_argb },
            FontSize { 9_pt },
            TextColor { 0x40000000_argb },
            Margin { 0 },
            Padding { 1_dp },
        },

        /// [ARadioButton]
        {
            t<ARadioButton>(), // styles the radiobutton + contents container
            LayoutSpacing { 4_dp }, // specifies space between ARadioButton::Circle and contents
        },
        {
            t<ARadioButton::Circle>(), // styles the circle itself
            BackgroundSolid { 0xffffff_rgb },
            Border { 1_dp, 0x333333_rgb },
            FixedSize { 14_dp, 14_dp },
            BorderRadius { 7_dp },
            BackgroundImage { {}, 0x333333_rgb },
        },
        {
            t<ARadioButton>::active() >> t<ARadioButton::Circle>(),
            BackgroundSolid { AColor::GRAY.transparentize(0.8f) },
        },
        {
            Selected(t<ARadioButton::Circle>()),
            BackgroundImage { ":uni/svg/radio.svg", getOsThemeColor().readableBlackOrWhite() },
            Border { nullptr },
            BackgroundGradient { getOsThemeColorLighter(), getOsThemeColor(), 180_deg },
        },
        {
            Selected(t<ARadioButton::Circle>::active()),
            BackgroundSolid { AColor::GRAY.transparentize(0.8f) },
            BackgroundGradient { getOsThemeColorLighter(), getOsThemeColor(), 180_deg },
        },
        {
            t<ARadioButton::Circle>::disabled(),
            BackgroundSolid { 0xe5e5e5_rgb },
            Border { 1_px, 0xa0a0a0_rgb },
        },
        {
            Selected(t<ARadioButton>::disabled()),
            BackgroundGradient { AColor::GRAY.lighter(0.1f), AColor::GRAY.darker(0.1f), 0_deg },
        },
        /// [ARadioButton]

        // AListView
        {
            {t<AListView>(), t<ATreeView>()},
            BackgroundSolid { 0xffffff_rgb },
            Border { 1_dp, 0x828790_rgb },
            Padding { 2_dp },
            Expanding { 0, 1 },
            MinSize { 120_dp, 80_dp },
            AOverflow::HIDDEN,
        },
        {
            t<ATreeView>() > t<AViewContainer>() > c(".list-item") > t<AAbstractLabel>(),
            Padding { 0, 2_dp },
            Margin { 0 },
        },
        {
            c(".list-item-icon"),
            MinSize { 7_pt, 7_pt },
            Margin { 2_dp, {} },
        },
        {
            c(".list-item-group"),
            Padding { 0, 0, 0, 4_dp },
            Margin { 0, 0, 4_dp, 15_dp },
            BorderLeft { 1_dp, 0x0_rgb },
        },
        {
            c(".list-item"),
            Margin { 0 },
            Padding { 1_dp, 4_dp, 4_dp },
        },
        {
            c::hover(".list-item"),
            BackgroundSolid { 0xe5f3ff_rgb },
        },
        {
            Selected(c(".list-item")),
            BackgroundSolid { 0xcde8ff_rgb },
        },

        // ADividerView
        {
            t<AHDividerView>(),
            FixedSize { {}, 1_px },
            BackgroundSolid { 0x808080_rgb },
        },
        {
            t<AVDividerView>(),
            FixedSize { 1_px, {} },
            BackgroundSolid { 0x808080_rgb },
        },

        // CUSTOM WINDOWS ===================================================
        {
            type_of<ACustomCaptionWindow>(),
            Padding { 0 },
            Border { 1_px, getOsThemeColor() },
        },
        {
            class_of(".window-title"),
            BackgroundSolid { getOsThemeColor() },
            FixedSize { {}, 30_dp }
        },
        {
            class_of(".window-title") >> class_of(".title"),
            Margin { 0 },
            Padding { 7_dp },
            TextColor { 0xffffff_rgb },
            FontSize { 9_pt },
        },
        {
            class_of(".window-title") >> class_of(".default"),
            BorderRadius{ 0 },
            BoxShadow { nullptr },
            Border { nullptr },
            Margin { 0 },
            Padding { 0 },
            MinSize { 45_dp, 29_dp },
            BackgroundSolid { nullptr },
            BackgroundImage {{}, {}, {}, Sizing::CENTER }
        },
        {
            class_of(".window-title") >> class_of(".minimize"),
            BackgroundImage {":uni/caption/minimize.svg", {}, {}, Sizing::CENTER }
        },
        {
            class_of(".window-title") >> class_of(".close"),
            BackgroundImage {":uni/caption/close.svg", {}, {}, Sizing::CENTER }
        },
        {
            class_of(".window-title") >> t<AButton>::hover(),
            BackgroundSolid { 0x20ffffff_argb },
        },
        {
            class_of(".window-title") >> t<AButton>::active(),
            BackgroundSolid { 0x30ffffff_argb },
        },
        {
            class_of(".window-title") >> class_of::hover(".close"),
            BackgroundSolid { 0xe81123_rgb }
        },
        {
            class_of(".window-title") >> class_of::active(".close"),
            BackgroundSolid { 0x80e81123_argb }
        },
        {
            class_of(".window-content"),
            BackgroundSolid { 0xffffff_rgb },
            MinSize { 200_dp, {} },
            Padding { 4_dp, 3_dp },
        },

        // ATabView
        {
            t<ATabButtonView>(),
            FontSize { 12_pt },
            TextColor { 0x444444_rgb },
            Padding { 8_dp, 4_dp },
        },
        {
            Selected(t<ATabButtonView>()),
            BorderBottom { 2_dp, getOsThemeColor() },
        },
        {
            t<ATabButtonView>::hover(),
            BackgroundSolid {0xffffff_rgb },
        },
        // scrollbar
        {
            t<AScrollAreaViewport>(),
            AOverflow::HIDDEN,
            Expanding(),
        },
        {
            t<AScrollbar>(),
            Margin { 0, 0, 0, 2_px },
        },
        {
            t<AScrollbarHandle>(),
#if AUI_PLATFORM_IOS || AUI_PLATFORM_ANDROID
            FixedSize { 6_dp, {} },
            BorderRadius { 3_dp },
#else
            FixedSize {{},{} },
#endif

            MinSize { 15_dp, 15_dp },
            BackgroundSolid { 0xcccccc_rgb },
            Margin {1_px, 1_px },
        },
        {
            t<AScrollbar>::disabled() > t<AScrollbarHandle>(),
            Visibility::GONE,
        },
        {
            t<AScrollbarButton>(),
#if AUI_PLATFORM_IOS || AUI_PLATFORM_ANDROID
            Visibility::GONE,
#endif
            FixedSize { 15_dp, 15_dp },
            BackgroundImage { {}, 0x404040_rgb },
            Margin {0 },
        },
        {
            t<AScrollbarButton>::disabled(),
            BackgroundSolid { 0 },
            BackgroundImage { {}, 0x707070_rgb },
        },
        {
            {t<AScrollbarHandle>::hover(), t<AScrollbarButton>::hover() },
            BackgroundSolid { 0xc0c0c0_rgb },
        },
        {
            c(".scrollbar_right"),
            ass::BackgroundImage {
                    ":uni/svg/sb-right.svg"
            },
            ass::FixedSize { 17_dp, 15_dp },
        },
        {
            c(".scrollbar_left"),
            ass::BackgroundImage {
                    ":uni/svg/sb-left.svg"
            },
            ass::FixedSize { 17_dp, 15_dp },
        },
        {
            c(".scrollbar_up"),
            ass::BackgroundImage {
                    ":uni/svg/sb-up.svg"
            },
            ass::FixedSize { 15_dp, 17_dp },
        },
        {
            c(".scrollbar_down"),
            ass::BackgroundImage {
                    ":uni/svg/sb-down.svg"
            },
            ass::FixedSize { 15_dp, 17_dp },
        },

        // menu
        {
            c(".menu-background"),
            BackgroundSolid {0xf0f0f0_rgb},
            Padding { 4_dp },
            BoxShadow { 0, 2_dp, 17_dp, 0xaa000000_argb },
        },
        {
            c(".menu"),
            Padding { 1_px },
            Margin { 0 },
            MinSize { 200_dp, 0 },
            Border { 1_px, 0x606060_rgb, }
        },
        {
            c(".menu-item"),
            Padding { 5_dp },
            Margin { 1_dp },
        },
        {
            c(".menu-separator"),
            Margin { 4_dp, 0 },
            FixedSize { {}, 1_px },
            BackgroundSolid { 0x606060_rgb },
        },
        {
            c::hover(".menu-item"),
            BackgroundSolid { 0xffffff_rgb },
        },
        {
            c(".menu-item-shortcut"),
            TextColor { 0x404040_rgb },
        },

        {
            c::disabled(".menu-item"),
            BackgroundSolid { 0x0_argb },
            Opacity { 0.7f },
        },

        /// [AProgressBar]
        {
            t<AProgressBar>(),
            BackgroundSolid { 0xd0d0d0_rgb },
            BorderRadius { 4_dp },
            MinSize { 40_dp, 8_dp },
        },
        {
            t<AProgressBar::Inner>(),
            BackgroundSolid { getOsThemeColor() },
            BorderRadius { 4_dp },
        },
        /// [AProgressBar]

        // ACircleProgressBar
        {
            t<ACircleProgressBar>(),
            Border { 4_dp, 0xd0d0d0_rgb },
            BorderRadius { 8_dp },
            FixedSize { 16_dp },
        },
        {
            t<ACircleProgressBar::Inner>(),
            Border { 4_dp, getOsThemeColor() },
            FixedSize { 16_dp },
            BorderRadius { 8_dp },
        },

        {
            t<ADrawableView>(),
            MinSize { 12_dp },
        },
        {
            t<ADrawableIconView>(),
            BackgroundImage { {}, {}, {}, Sizing::CONTAIN },
        },

        // AGroupBox
        {
          t<AGroupBox>(),
          LayoutSpacing { -6_pt /* half of the title's height */ }
        },
        {
            c(".agroupbox-title"),
            Margin { {}, 4_dp },
        },
        {
            c(".agroupbox-title") > t<AView>(),
            Padding { {}, 4_dp },
        },
        {
            c(".agroupbox-inner"),
            Border { 2_dp, 0x30808080_argb },
            BorderRadius { 4_dp },
            Padding { 10_dp, 8_dp },
        },
        {
            c(".modal-scaffold-dim"),
            BackgroundSolid { 0x40000000_argb },
            Expanding{},
        },
    });
}

AColor AStylesheet::getOsThemeColor() {
#if AUI_PLATFORM_WIN
    auto impl = []() {
        DWORD c = 0;
        BOOL blending;
        DwmGetColorizationColor(&c, &blending);
        c |= 0xff000000;
        AColor osThemeColor = AColor::fromAARRGGBB(static_cast<unsigned>(c));
        float readability = osThemeColor.readabilityOfForegroundColor(0xffffffff);
        if (readability < 0.3f) {
            osThemeColor = osThemeColor.darker(1.f - readability * 0.5f);
        }
        return osThemeColor;
    };
    static AColor osThemeColor = impl();

    return osThemeColor;
#elif AUI_PLATFORM_LINUX
    return 0x1a6acb_rgb; // adwaita accent color
#elif AUI_PLATFORM_APPLE
    return 0x057aff_rgb;
#else
    return 0x1a6acb_rgb;
#endif
}

AStylesheet& AStylesheet::global() { return *globalStorage(); }

_<AStylesheet>& AStylesheet::globalStorage() {
    static _<AStylesheet> s = _new<AStylesheet>();
    return s;
}
