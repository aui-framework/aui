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
// Created by alex2 on 29.12.2020.
//

#if AUI_PLATFORM_WIN
#include <dwmapi.h>
#endif

#include <AUI/View/AButton.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/View/AAbstractTextField.h>
#include <AUI/View/ANumberPicker.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/AComboBox.h>
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
#include "AStylesheet.h"
#include "ASS.h"
#include "AUI/View/ASpinner.h"


AStylesheet::AStylesheet() {
    using namespace ass;

    addRules({
        // COMMON VIEWS ======================================
        // AView
        {
            t<AView>(),
            TextColor { 0x0_rgb },
            FontSize {9_pt},
            FontRendering::SUBPIXEL,
            ImageRendering::SMOOTH,
            MaxSize {99999999_dp, 99999999_dp },
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
            Margin { 4_dp },
            FixedSize { 16_dp }
        },

        // AText
        {
            t<AText>(),
            Margin { 2_dp, 4_dp },
            LineHeight { 1.f },
            VerticalAlign::MIDDLE,
        },
        // AAbstractLabel
        {
            t<AAbstractLabel>(),
            Padding {1_dp, 0, 2_dp},
            VerticalAlign::MIDDLE,
        },
        {
            t<AAbstractLabel>::disabled(),
            TextColor { 0x444444_rgb },
        },
        {
            t<AAbstractLabel>(),
            Margin {2_dp, 4_dp},
        },

        // AButton
        {
            { t<AButton>(), c(".btn")},
            BackgroundSolid {0xffffff_rgb},
            Padding {3_dp, 6_dp},
            Margin {2_dp, 4_dp},
            MinSize {60_dp, {} },
            Border { 1_dp, 0xcacaca_rgb },
            BorderRadius {4_dp},
            TextAlign::CENTER,
            VerticalAlign::MIDDLE,
            BoxShadow {{}, 1_dp, 4_dp, -2_dp, 0x80000000_argb},
        },
        {
            { t<AButton>::hover(), c::hover(".btn")},
            Border {1_dp, getOsThemeColor() * glm::vec4(1, 1, 1, 0.3f)}
        },
        {
            { t<AButton>::active(), c::active(".btn")},
            BackgroundSolid{0xfafafa_rgb},
        },
        {
            t<AButtonEx>(),
            LayoutSpacing { 2_dp },
            MinSize { 2_dp },
        },
        {
            t<AButtonEx>() > t<ALabel>(),
            Margin { 0 },
            Padding { 0 },
        },
        {
            { debug_selector(), button::Default(t<AButton>()), c(".btn_default")},
            FontRendering::ANTIALIASING,
            BackgroundGradient {getOsThemeColor().lighter(0.15f),
                                getOsThemeColor().darker(0.15f),
                                LayoutDirection::VERTICAL },
            BoxShadow { 0, 1_dp, 3_dp, -1_dp, getOsThemeColor() },
            Border { nullptr },
            TextColor { 0xffffff_rgb },
        },
        {
            { button::Default(t<AButton>::hover()), c::hover(".btn_default")},
            BoxShadow { 0, 1_dp, 6_dp, -1_dp, getOsThemeColor() },
        },
        {
            { t<AButton>::active(), c::active(".btn")},
            Padding { 4_dp, {}, 2_dp },
            BoxShadow { nullptr },
        },
        {
            { button::Default(t<AButton>::hover()), c::hover(".btn_default")},
            BackgroundGradient {getOsThemeColor().lighter(0.2f),
                                getOsThemeColor().darker(0.15f),
                                LayoutDirection::VERTICAL },
        },
        {
            { button::Default(t<AButton>::active()), c::active(".btn_default")},
            BackgroundSolid { getOsThemeColor() }
        },
        {
            { t<AButton>::disabled(), c::disabled(".btn") },
            BackgroundSolid { 0xcccccc_rgb },
            BoxShadow { nullptr },
            Border { 1_dp, 0xbfbfbf_rgb },
            TextColor { 0x838383_rgb }
        },

        // Text fields
        {
            t<AAbstractTypeableView>(),
            Padding { 3_dp, 6_dp },
            ACursor::TEXT,
        },
        {
            class_of(".input-field"),
            TextColor { 0_rgb },
            BackgroundSolid { 0xffffff_rgb },
            Border { 1_dp, 0xa0a0a0_rgb },
            BorderRadius { 4_dp },
            Margin { 2_dp, 4_dp },
            MinSize { 100_dp, 17_dp },
            Overflow::HIDDEN
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
            t<ANumberPicker>(),
            MinSize { 60_dp, {} },
            Overflow::HIDDEN,
        },
        {
            class_of(".up-down-wrapper"),
            Padding { 1_px },
            Margin { 0 },
            LayoutSpacing { 1_px },
            //BackgroundSolid { 0xf0f0f0_rgb }
        },
        {
            t<ANumberPicker>() >> t<AButton>(),
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
            t<ANumberPicker>() >> c(".up"),
            BackgroundImage { ":uni/svg/up.svg", {} },
        },
        {
            t<ANumberPicker>() >> c(".down"),
            BackgroundImage { ":uni/svg/down.svg", {} },
        },

        // ACheckBox
        {
            t<ACheckBox>(),
            Margin { 1_dp, 4_dp },
        },
        {
            t<ACheckBox>() > t<AAbstractLabel>(),
            Margin { 1_dp, 4_dp },
        },
        {
            t<ACheckBoxInner>(),
            BackgroundSolid { 0xffffff_rgb },
            Margin { 1_dp, 1_dp, 1_dp, 4_dp },
            Border { 1_dp, 0x333333_rgb },
            FixedSize { 13_dp, 13_dp },
            BackgroundImage {0x333333_rgb },
        },
        {
            t<ACheckBoxInner>::hover(),
            Border { 1_dp, 0x0078d7_rgb },
            BackgroundImage {0x0078d7_rgb },
        },
        {
            t<ACheckBoxInner>::hover(),
            BackgroundSolid { 0xcce4f7_rgb },
            Border { 1_dp, 0x005499_rgb },
            BackgroundImage {0x005499_rgb },
        },
        {
            Selected(t<ACheckBox>()) >> t<ACheckBoxInner>(),
            BackgroundImage {":uni/svg/checkbox.svg" },
        },
        {
            t<ACheckBox>::disabled() > t<AAbstractLabel>(),
            TextColor { 0xa0a0a0_rgb },
        },
        {
            t<ACheckBox>::disabled() >> t<ACheckBoxInner>(),
            BackgroundSolid { 0xe5e5e5_rgb },
            BackgroundImage { 0xa0a0a0_rgb },
            Border { 1_px, 0xa0a0a0_rgb },
        },

        // ARulerView
        {
            t<ARulerView>(),
            MinSize { 16_dp },
            BackgroundSolid { 0x20000000_argb },
            TextColor { 0x40000000_argb },
            Overflow::HIDDEN,
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

        // ARadioButton
        {
            t<ARadioButton>(),
            Margin { 1_dp, 4_dp },
        },
        {
            t<ARadioButton>() > t<AAbstractLabel>(),
            Margin { 1_dp, 4_dp },
        },
        {
            t<ARadioButtonInner>(),
            BackgroundSolid { 0xffffff_rgb },
            Margin { 3_dp, 1_dp, 1_dp, 4_dp },
            Border { 1_dp, 0x333333_rgb },
            FixedSize { 13_dp, 13_dp },
            BorderRadius { 6_dp },
            BackgroundImage {0x333333_rgb },
        },
        {
            t<ARadioButtonInner>::hover(),
            Border { 1_dp, 0x0078d7_rgb },
            BackgroundImage {0x0078d7_rgb },
        },
        {
            t<ARadioButtonInner>::hover(),
            BackgroundSolid { 0xcce4f7_rgb },
            Border { 1_dp, 0x005499_rgb },
            BackgroundImage {0x005499_rgb },
        },
        {
                Selected(t<ARadioButton>()) > t<ARadioButtonInner>(),
            BackgroundImage {":uni/svg/radio.svg" },
        },
        {
            t<ARadioButton>::disabled() > t<AAbstractLabel>(),
            TextColor { 0xa0a0a0_rgb },
        },
        {
            t<ARadioButton>::disabled() > t<ARadioButtonInner>(),
            BackgroundSolid { 0xe5e5e5_rgb },
            BackgroundImage { 0xa0a0a0_rgb },
            Border { 1_dp, 0xa0a0a0_rgb },
        },

        // AComboBox
        {
            t<AComboBox>(),
            TextAlign::LEFT,
        },

        // AListView
        {
            {t<AListView>(), t<ATreeView>()},
            BackgroundSolid { 0xffffff_rgb },
            Border { 1_dp, 0x828790_rgb },
            Padding { 2_dp },
            Margin {2_dp, 4_dp},
            Expanding { 0, 1 },
            MinSize { 120_dp, 80_dp },
            Overflow::HIDDEN,
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
            {t<AListView>() > t<AViewContainer>() > t<AAbstractLabel>(), c(".list-item")},
            Margin { 0 },
            Padding { 1_dp, 4_dp, 4_dp },
        },
        {
            {t<AListView>() > t<AViewContainer>() > t<AAbstractLabel>::hover(), c::hover(".list-item"),},
            BackgroundSolid { 0xe5f3ff_rgb },
        },
        {
            {t<AListView>() > t<AViewContainer>() > Selected(t<AAbstractLabel>()), Selected(c(".list-item"))},
            BackgroundSolid { 0xcde8ff_rgb },
        },

        // ADividerView
        {
            t<AHDividerView>(),
            FixedSize { {}, 2_dp },
            Margin { 0, 2_dp },
            BackgroundSolid { 0x808080_rgb },
        },
        {
            t<AVDividerView>(),
            FixedSize { 2_dp, {} },
            Margin { 2_dp, 0 },
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
            TextColor { 0xffffff_rgb }
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

        // scroll area
        {
            c(".scrollarea_inner"),
            Overflow::HIDDEN
        },
        // scrollbar
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
            FixedSize { 15_dp, {} },
#endif

            MinSize { {}, 40_dp },
            BackgroundSolid { 0xcccccc_rgb },
            Margin {1_px, 0},
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

        // AProgressBar
        {
            t<AProgressBar>(),
            BackgroundSolid { 0xd0d0d0_rgb },
            BorderRadius { 4_dp },
            FixedSize { {}, 8_dp },
            Margin { 2_dp, 4_dp },
        },
        {
            t<AProgressBar::Inner>(),
            BackgroundSolid { getOsThemeColor() },
            BorderRadius { 4_dp },
        },

        {
            t<ADrawableView>(),
            MinSize { 12_dp },
        }
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
        if (readability < 0.3f)  {
            osThemeColor = osThemeColor.darker(1.f - readability * 0.5f);
        }
        return osThemeColor;
    };
    static AColor osThemeColor = impl();

    return osThemeColor;
#else
    return 0x3e3e3e_rgb;
#endif
}

AStylesheet& AStylesheet::global() {
    return *globalStorage();
}

_<AStylesheet>& AStylesheet::globalStorage() {
    static _<AStylesheet> s = _new<AStylesheet>();
    return s;
}
