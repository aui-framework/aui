//
// Created by alex2 on 29.12.2020.
//

#include <AUI/View/AButton.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/View/AAbstractTextField.h>
#include <AUI/View/ANumberPicker.h>
#include <AUI/View/ACheckBox.h>
#include "AStylesheet.h"
#include "ASS.h"
#ifdef _WIN32
#include <dwmapi.h>
#endif

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
            AView::OF_HIDDEN
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
            FixedSize {
                .width = 19_dp
            },
            MinSize {
                .height = 9_dp
            }
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
            Border { 1_dp, 0x333333_rgb },
            FixedSize { 13_dp, 13_dp },
        },

        // CUSTOM WINDOWS ===================================================
        {
            class_of(".window-title"),
            BackgroundSolid { getOsThemeColor() },
            FixedSize { .height = 30_dp }
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
            BackgroundSolid{ nullptr }
        },
        {
            class_of(".window-title") >> class_of(".minimize"),
            BackgroundUrl { ":win/caption/minimize.svg" }
        },
        {
            class_of(".window-title") >> class_of(".close"),
            BackgroundUrl { ":win/caption/close.svg" }
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
            MinSize {
                .width = 200_dp
            },
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
