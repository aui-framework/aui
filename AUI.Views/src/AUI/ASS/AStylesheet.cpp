//
// Created by alex2 on 29.12.2020.
//

#include <AUI/View/AButton.h>
#include <AUI/Platform/AWindow.h>
#include "AStylesheet.h"
#include "ASS.h"
#ifdef _WIN32
#include <dwmapi.h>
#endif

AStylesheet::AStylesheet() {
    using namespace ass;
    
    addRules({
        // COMMON VIEWS ======================================
        {
            any<AView>(),
            FontSize {9_pt},
            FontRendering::SUBPIXEL,
        },
        {
            any<AWindow>(),
            SolidBackground {0xf0f0f0_rgb},
        },
        {
            any<ALabel>(),
            Padding {1_dp, {}, 2_dp},
        },
        {
            any<ALabel>(),
            Margin {2_dp, 4_dp},
        },
        {
            any<AButton>(),
            SolidBackground {0xffffff_rgb},
            Padding {4_dp, 8_dp},
            Margin {2_dp, 4_dp},
            MinSize {60_dp, 14_dp},
            BorderRadius {4_dp},
            TextAlign::CENTER,
            BoxShadow {{}, 1_dp, 5_dp, -2_dp, 0x80000000_argb},
        },
        {
            any<AButton>::hover(),
            Border {1_dp, getOsThemeColor() * glm::vec4(1, 1, 1, 0.3f)}
        },
        {
            any<AButton>::active(),
            SolidBackground{0xfafafa_rgb},
        },
        {
            any<AButton>()["default"],
            FontRendering::ANTIALIASING,
            GradientBackground { getOsThemeColor().lighter(0.15f),
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
            GradientBackground { getOsThemeColor().lighter(0.2f),
                                 getOsThemeColor().darker(0.15f),
                                 LayoutDirection::VERTICAL },
        },
        {
            any<AButton>::active()["default"],
            SolidBackground { getOsThemeColor() }
        },
        {
            any<AButton>()["disabled"],
            SolidBackground { 0xcccccc_rgb },
            BoxShadow { nullptr },
            Border {1_dp, 0xbfbfbf_rgb },
            TextColor { 0x838383_rgb }
        },

        // CUSTOM VIEWS ===================================================
        {
            class_of(".window-title"),
            SolidBackground { getOsThemeColor() },
            FixedSize { .height = 30_dp }
        }
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
    return 0xff2147_rgb;
#endif
}
