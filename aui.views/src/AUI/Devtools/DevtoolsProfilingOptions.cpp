/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "DevtoolsProfilingOptions.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/AText.h"
#include "AUI/Platform/APlatform.h"

using namespace ass;
using namespace declarative;

namespace {
    _<AView> coloredRect(AColor c) {
        return Centered {
            _new<AView>() with_style {
                BackgroundSolid { c },
                FixedSize { 10_pt },
            }
        };
    }

    _<AView> makeLink(AString text, AUrl destination) {
        _<AView> l = Label{std::move(text)};
        AObject::connect(l->clicked, l, [destination = std::move(destination)] {
            APlatform::openUrl(destination);
        });
        l->setCustomStyle({
            Margin { 0 },
            Padding { 1_dp },
            ACursor::POINTER,
            BorderBottom{1_px, AColor::BLACK}
        });
        return l;
    }
}

DevtoolsProfilingOptions::DevtoolsProfilingOptions(ABaseWindow* targetWindow): mModel(&targetWindow->profiling()) {

    setContents(Centered {
        Vertical::Expanding {
            AText::fromItems({"These settings are applicable for render-to-texture optimizations.",
                              makeLink("Learn more", "https://aui-framework.github.io/develop/md_docs_Render_to_texture.html")}),
            CheckBoxWrapper { Label { "Highlight redraw requests" } } && mModel(&ABaseWindow::Profiling::highlightRedrawRequests),
            AText::fromItems({"Draws purple rect ", coloredRect(0xff00ff_rgb), " over view that requested redraw "
                              "(AView::redraw). This can help to find views that causes window to repaint which "
                              "affects application performance and device's battery life."}),
            CheckBoxWrapper { Label { "Render to texture decay" } } && mModel(&ABaseWindow::Profiling::renderToTextureDecay),
            AText::fromItems({"Visually displays render to texture optimization by gradually transforming old pixel ",
                              "data to gray ", coloredRect(AColor::GRAY), " color. When a view is redrawn it pixel "
                              "data would be represented as unaffected to gray color and seem bright and saturated "
                              "color. From perspective of performance it's good that whole screen transformed to gray "
                              "color and thus no redraw is performed."
                                }),
            CheckBoxWrapper { Label { "Breakpoint on AWindow update layout flag" } } && mModel(&ABaseWindow::Profiling::breakpointOnMarkMinContentSizeInvalid),
            AText::fromItems({"Stops the attached debugger at the point when window's update layout flag is set. This "
                              "can be used to walk through stacktrace and find which view and why triggered layout "
                              "update. When breakpoint is triggered, checkbox is unset. Note: when debugger is not attached, behaviour is undefined."}),
        } with_style {
            MaxSize { 700_dp, {} },
        }
    });
    setExtraStylesheet({
       {
           t<ACheckBoxWrapper>(),
           BackgroundSolid { AColor::GRAY.transparentize(0.8f) },
           BorderRadius { 16_dp },
           Padding { 8_dp, 16_dp },
           Margin { 8_dp },
       },
       {
           t<AText>(),
           Padding { 0 },
           Margin { 4_dp, 24_dp, 8_dp },
           Opacity { 0.5f },
       }
    });
    connect(targetWindow->redrawn, [this] {
        mModel.notifyUpdate();
    });
}
