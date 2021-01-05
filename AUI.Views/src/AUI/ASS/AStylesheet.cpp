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
            any<AView>(),
            FontSize {9_pt},
            FontRendering::SUBPIXEL,
        },

        // AWindow
        {
            any<AWindow>(),
            BackgroundSolid {0xf0f0f0_rgb},
            Padding { 4_dp },
        },

        // ALabel
        {
            any<ALabel>(),
            Padding {1_dp, {}, 2_dp},
        },
        {
            any<ALabel>(),
            Margin {2_dp, 4_dp},
        },

        // AButton
        {
            any<AButton>(),
            BackgroundSolid {0xffffff_rgb},
            Padding {4_dp, 8_dp},
            Margin {2_dp, 4_dp},
            MinSize {60_dp, 14_dp},
            BorderRadius {4_dp},
            TextAlign::CENTER,
            BoxShadow {{}, 1_dp, 4_dp, -2_dp, 0x80000000_argb},
        },
        {
            any<AButton>::hover(),
            Border {1_dp, getOsThemeColor() * glm::vec4(1, 1, 1, 0.3f)}
        },
        {
            any<AButton>::active(),
            BackgroundSolid{0xfafafa_rgb},
        },
        {
            any<AButton>()["default"],
            FontRendering::ANTIALIASING,
            BackgroundGradient {getOsThemeColor().lighter(0.15f),
                                getOsThemeColor().darker(0.15f),
                                LayoutDirection::VERTICAL },
            BoxShadow { {}, 1_dp, 3_dp, -1_dp, getOsThemeColor() },
            Border { nullptr },
            TextColor { 0xffffff_rgb },
        },
        {
            any<AButton>::hover()["default"],
            BoxShadow { {}, 1_dp, 6_dp, -1_dp, getOsThemeColor() },
        },
        {
            any<AButton>::active(),
            Padding {5_dp, 8_dp, 3_dp},
            BoxShadow { nullptr },
        },
        {
            any<AButton>::hover()["default"],
            BackgroundGradient {getOsThemeColor().lighter(0.2f),
                                getOsThemeColor().darker(0.15f),
                                LayoutDirection::VERTICAL },
        },
        {
            any<AButton>::active()["default"],
            BackgroundSolid { getOsThemeColor() }
        },
        {
            any<AButton>()["disabled"],
            BackgroundSolid { 0xcccccc_rgb },
            BoxShadow { nullptr },
            Border {1_dp, 0xbfbfbf_rgb },
            TextColor { 0x838383_rgb }
        },

        // Text fields
        {
            any<AAbstractTextField>(),
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
            any<ANumberPicker>() >> any<AButton>(),
            Margin { {} },
            Padding { {} },
            FixedSize { 19_dp, {} },
            MinSize { {}, 9_dp }
        },

        // ACheckBox
        {
            any<ACheckBox>(),
            Margin { 1_dp, 4_dp },
        },
        {
            any<ACheckBox>() > any<ALabel>(),
            Margin { 1_dp, 4_dp },
        },
        {
            any<ACheckBoxInner>(),
            BackgroundSolid { 0xffffff_rgb },
            Margin { 1_dp, 1_dp, 1_dp, 4_dp },
            Border { 1_px, 0x333333_rgb },
            FixedSize { 13_dp, 13_dp },
            BackgroundImage {0x333333_rgb },
        },
        {
            any<ACheckBoxInner>::hover(),
            Border { 1_px, 0x0078d7_rgb },
            BackgroundImage {0x0078d7_rgb },
        },
        {
            any<ACheckBoxInner>::hover(),
            BackgroundSolid { 0xcce4f7_rgb },
            Border { 1_px, 0x005499_rgb },
            BackgroundImage {0x005499_rgb },
        },
        {
            any<ACheckBox>()["checked"] > any<ACheckBoxInner>(),
            BackgroundImage {":uni/svg/checkbox.svg" },
        },
        {
            any<ACheckBox>()["disabled"] > any<ALabel>(),
            TextColor { 0xa0a0a0_rgb },
        },
        {
            any<ACheckBox>()["disabled"] > any<ACheckBoxInner>(),
            BackgroundSolid { 0xe5e5e5_rgb },
            BackgroundImage { 0xa0a0a0_rgb },
            Border { 1_px, 0xa0a0a0_rgb },
        },

        // ARadioButton
        {
            any<ARadioButton>(),
            Margin { 1_dp, 4_dp },
        },
        {
            any<ARadioButton>() > any<ALabel>(),
            Margin { 1_dp, 4_dp },
        },
        {
            any<ARadioButtonInner>(),
            BackgroundSolid { 0xffffff_rgb },
            Margin { 1_dp, 1_dp, 1_dp, 4_dp },
            Border { 1_px, 0x333333_rgb },
            FixedSize { 13_dp, 13_dp },
            BorderRadius { 6_dp },
            BackgroundImage {0x333333_rgb },
        },
        {
            any<ARadioButtonInner>::hover(),
            Border { 1_px, 0x0078d7_rgb },
            BackgroundImage {0x0078d7_rgb },
        },
        {
            any<ARadioButtonInner>::hover(),
            BackgroundSolid { 0xcce4f7_rgb },
            Border { 1_px, 0x005499_rgb },
            BackgroundImage {0x005499_rgb },
        },
        {
            any<ARadioButton>()["checked"] > any<ARadioButtonInner>(),
            BackgroundImage {":uni/svg/radio.svg" },
        },
        {
            any<ARadioButton>()["disabled"] > any<ALabel>(),
            TextColor { 0xa0a0a0_rgb },
        },
        {
            any<ARadioButton>()["disabled"] > any<ARadioButtonInner>(),
            BackgroundSolid { 0xe5e5e5_rgb },
            BackgroundImage { 0xa0a0a0_rgb },
            Border { 1_px, 0xa0a0a0_rgb },
        },

        // AComboBox
        {
            any<AComboBox>(),
            TextAlign::LEFT,
        },

        // AListView
        {
            any<AListView>(),
            BackgroundSolid { 0xffffff_rgb },
            Border { 1_px, 0x828790_rgb },
            Padding { 2_px },
            Overflow::HIDDEN,
        },
        {
            any<AListView>() > any<ALabel>(),
            Margin { {} },
            Padding { 1_px, 4_px, 4_px },
        },
        {
            any<AListView>() > any<ALabel>::hover(),
            BackgroundSolid { 0xe5f3ff_rgb },
        },
        {
            any<AListView>() > any<ALabel>()["selected"],
            BackgroundSolid { 0xcde8ff_rgb },
        },

        // CUSTOM WINDOWS ===================================================
        {
            class_of(".window-title"),
            BackgroundSolid { getOsThemeColor() },
            FixedSize { {}, 30_dp }
        },
        {
            class_of(".window-title") >> any<ALabel>(),
            Margin { {} },
            Padding { 7_dp },
            TextColor { 0xffffff_rgb }
        },
        {
            class_of(".window-title") >> class_of(".default"),
            BorderRadius{ {} },
            BoxShadow { nullptr },
            Border { nullptr },
            Margin { {} },
            Padding { {} },
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
            class_of(".window-title") >> any<AButton>::hover(),
            BackgroundSolid { 0x20ffffff_argb },
        },
        {
            class_of(".window-title") >> any<AButton>::active(),
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
