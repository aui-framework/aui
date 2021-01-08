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

//
// Created by alex2 on 29.12.2020.
//

#ifdef _WIN32
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
#include "AStylesheet.h"
#include "ASS.h"


AStylesheet::AStylesheet() {
    using namespace ass;

    addRules({
        // COMMON VIEWS ======================================
        // AView
        {
                type_of<AView>(),
            FontSize {9_pt},
            FontRendering::SUBPIXEL,
        },

        // AWindow
        {
                type_of<AWindow>(),
            BackgroundSolid {0xf0f0f0_rgb},
            Padding { 4_dp },
        },

        // ALabel
        {
                type_of<ALabel>(),
            Padding {1_dp, 0, 2_dp},
        },
        {
                type_of<ALabel>(),
            Margin {2_dp, 4_dp},
        },

        // AButton
        {
                type_of<AButton>(),
            BackgroundSolid {0xffffff_rgb},
            Padding {4_dp, 8_dp},
            Margin {2_dp, 4_dp},
            MinSize {60_dp, 14_dp},
            BorderRadius {4_dp},
            TextAlign::CENTER,
            BoxShadow {{}, 1_dp, 4_dp, -2_dp, 0x80000000_argb},
        },
        {
                type_of<AButton>::hover(),
            Border {1_dp, getOsThemeColor() * glm::vec4(1, 1, 1, 0.3f)}
        },
        {
                type_of<AButton>::active(),
            BackgroundSolid{0xfafafa_rgb},
        },
        {
                type_of<AButton>()["default"],
            FontRendering::ANTIALIASING,
            BackgroundGradient {getOsThemeColor().lighter(0.15f),
                                getOsThemeColor().darker(0.15f),
                                LayoutDirection::VERTICAL },
            BoxShadow { 0, 1_dp, 3_dp, -1_dp, getOsThemeColor() },
            Border { nullptr },
            TextColor { 0xffffff_rgb },
        },
        {
                type_of<AButton>::hover()["default"],
            BoxShadow { 0, 1_dp, 6_dp, -1_dp, getOsThemeColor() },
        },
        {
                type_of<AButton>::active(),
            Padding {5_dp, 8_dp, 3_dp},
            BoxShadow { nullptr },
        },
        {
                type_of<AButton>::hover()["default"],
            BackgroundGradient {getOsThemeColor().lighter(0.2f),
                                getOsThemeColor().darker(0.15f),
                                LayoutDirection::VERTICAL },
        },
        {
                type_of<AButton>::active()["default"],
            BackgroundSolid { getOsThemeColor() }
        },
        {
                type_of<AButton>()["disabled"],
            BackgroundSolid { 0xcccccc_rgb },
            BoxShadow { nullptr },
            Border {1_dp, 0xbfbfbf_rgb },
            TextColor { 0x838383_rgb }
        },

        // Text fields
        {
                type_of<AAbstractTextField>(),
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

        // ANumberPicker
        {
            class_of(".up-down-wrapper"),
            Padding { 1_px },
            Margin { 1_px },
            LayoutSpacing { 1_px },
            BackgroundSolid { 0xf0f0f0_rgb }
        },
        {
                type_of<ANumberPicker>() >> type_of<AButton>(),
            Margin { 0 },
            Padding { 0 },
            FixedSize { 19_dp, {} },
            MinSize { {}, 9_dp }
        },

        // ACheckBox
        {
                type_of<ACheckBox>(),
            Margin { 1_dp, 4_dp },
        },
        {
                type_of<ACheckBox>() > type_of<ALabel>(),
            Margin { 1_dp, 4_dp },
        },
        {
                type_of<ACheckBoxInner>(),
            BackgroundSolid { 0xffffff_rgb },
            Margin { 1_dp, 1_dp, 1_dp, 4_dp },
            Border { 1_px, 0x333333_rgb },
            FixedSize { 13_dp, 13_dp },
            BackgroundImage {0x333333_rgb },
        },
        {
                type_of<ACheckBoxInner>::hover(),
            Border { 1_px, 0x0078d7_rgb },
            BackgroundImage {0x0078d7_rgb },
        },
        {
                type_of<ACheckBoxInner>::hover(),
            BackgroundSolid { 0xcce4f7_rgb },
            Border { 1_px, 0x005499_rgb },
            BackgroundImage {0x005499_rgb },
        },
        {
                type_of<ACheckBox>()["checked"] > type_of<ACheckBoxInner>(),
            BackgroundImage {":uni/svg/checkbox.svg" },
        },
        {
                type_of<ACheckBox>()["disabled"] > type_of<ALabel>(),
            TextColor { 0xa0a0a0_rgb },
        },
        {
                type_of<ACheckBox>()["disabled"] > type_of<ACheckBoxInner>(),
            BackgroundSolid { 0xe5e5e5_rgb },
            BackgroundImage { 0xa0a0a0_rgb },
            Border { 1_px, 0xa0a0a0_rgb },
        },

        // ARadioButton
        {
                type_of<ARadioButton>(),
            Margin { 1_dp, 4_dp },
        },
        {
                type_of<ARadioButton>() > type_of<ALabel>(),
            Margin { 1_dp, 4_dp },
        },
        {
                type_of<ARadioButtonInner>(),
            BackgroundSolid { 0xffffff_rgb },
            Margin { 1_dp, 1_dp, 1_dp, 4_dp },
            Border { 1_px, 0x333333_rgb },
            FixedSize { 13_dp, 13_dp },
            BorderRadius { 6_dp },
            BackgroundImage {0x333333_rgb },
        },
        {
                type_of<ARadioButtonInner>::hover(),
            Border { 1_px, 0x0078d7_rgb },
            BackgroundImage {0x0078d7_rgb },
        },
        {
                type_of<ARadioButtonInner>::hover(),
            BackgroundSolid { 0xcce4f7_rgb },
            Border { 1_px, 0x005499_rgb },
            BackgroundImage {0x005499_rgb },
        },
        {
                type_of<ARadioButton>()["checked"] > type_of<ARadioButtonInner>(),
            BackgroundImage {":uni/svg/radio.svg" },
        },
        {
                type_of<ARadioButton>()["disabled"] > type_of<ALabel>(),
            TextColor { 0xa0a0a0_rgb },
        },
        {
                type_of<ARadioButton>()["disabled"] > type_of<ARadioButtonInner>(),
            BackgroundSolid { 0xe5e5e5_rgb },
            BackgroundImage { 0xa0a0a0_rgb },
            Border { 1_px, 0xa0a0a0_rgb },
        },

        // AComboBox
        {
                type_of<AComboBox>(),
            TextAlign::LEFT,
        },

        // AListView
        {
                type_of<AListView>(),
            BackgroundSolid { 0xffffff_rgb },
            Border { 1_px, 0x828790_rgb },
            Padding { 2_px },
            Overflow::HIDDEN,
        },
        {
                type_of<AListView>() > type_of<ALabel>(),
            Margin { 0 },
            Padding { 1_px, 4_px, 4_px },
        },
        {
                type_of<AListView>() > type_of<ALabel>::hover(),
            BackgroundSolid { 0xe5f3ff_rgb },
        },
        {
                type_of<AListView>() > type_of<ALabel>()["selected"],
            BackgroundSolid { 0xcde8ff_rgb },
        },

        // CUSTOM WINDOWS ===================================================
        {
            class_of(".window-title"),
            BackgroundSolid { getOsThemeColor() },
            FixedSize { {}, 30_dp }
        },
        {
                class_of(".window-title") >> type_of<ALabel>(),
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
            BackgroundSolid { nullptr }
        },
        {
            class_of(".window-title") >> class_of(".minimize"),
            BackgroundImage {":uni/caption/minimize.svg" }
        },
        {
            class_of(".window-title") >> class_of(".close"),
            BackgroundImage {":uni/caption/close.svg" }
        },
        {
                class_of(".window-title") >> type_of<AButton>::hover(),
            BackgroundSolid { 0x20ffffff_argb },
        },
        {
                class_of(".window-title") >> type_of<AButton>::active(),
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
    });
}


AStylesheet& AStylesheet::inst() {
    static AStylesheet s;
    return s;
}

AColor AStylesheet::getOsThemeColor() {
#if defined(_WIN32)
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
