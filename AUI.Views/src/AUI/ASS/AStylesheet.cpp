//
// Created by alex2 on 29.12.2020.
//

#include <AUI/View/AButton.h>
#include <AUI/Platform/AWindow.h>
#include "AStylesheet.h"
#include "ASS.h"


AStylesheet::AStylesheet() {
    using namespace ass;
    
    addRules({
        {
            any<AView>(),
            FontSize {9_pt},
            FontRendering::SUBPIXEL,
        },
        {
            any<AWindow>(),
            SolidBackground {0xfff0f0f0_argb},
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
            SolidBackground {0xffffffff_argb},
            Padding {4_dp, 8_dp},
            Margin {2_dp, 4_dp},
            MinSize {60_dp, 14_dp},
            BorderRadius {4_dp},
            TextAlign::CENTER,
            BoxShadow {{}, 1_dp, 5_dp, -2_dp, 0x80000000_argb},
        },
        {
            any<AButton>::hover(),
            Border {1_dp, 0xff000000_argb}
        },
        {
            any<AButton>::active(),
            SolidBackground {0xfffafafa_argb},
            Padding {5_dp, 8_dp, 3_dp},
            BoxShadow {nullptr},
        }
    });
}


AStylesheet& AStylesheet::inst() {
    static AStylesheet s;
    return s;
}
