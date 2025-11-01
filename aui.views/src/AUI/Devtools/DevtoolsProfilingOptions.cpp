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

#include <AUI/View/ASlider.h>
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
        _new<AView>() AUI_WITH_STYLE {
          BackgroundSolid { c },
          FixedSize { 10_pt },
        }
    };
}

_<AView> makeLink(AString text, AUrl destination) {
    _<AView> l = Label { std::move(text) };
    AObject::connect(l->clicked, l, [destination = std::move(destination)] { APlatform::openUrl(destination); });
    l->setCustomStyle({ ACursor::POINTER, Margin { 0 }, Padding { 0 }, BorderBottom { 1_px, AColor::BLACK } });
    return l;
}

_<ALabel> header(AString title) {
    return Label { std::move(title) } AUI_WITH_STYLE { FontSize{16_pt}, Padding{0}, Margin { 4_dp, 24_dp, 8_dp } };
}
}   // namespace

DevtoolsProfilingOptions::DevtoolsProfilingOptions(AWindowBase* targetWindow) {
    auto scalingParams = _new<AProperty<aui::float_within_0_1>>(1.f / 3.f);
    setContents(Centered { AScrollArea::Builder().withContents(
        Vertical::Expanding {
          header("Profiling"),
          /// [fromItems]
          AText::fromItems(
              { "These settings are applicable for render-to-texture optimizations.",
                makeLink("Learn more", "https://aui-framework.github.io/develop/md_docs_Render_to_texture.html") }),
          CheckBox {
            AUI_REACT(*targetWindow->profiling()->highlightRedrawRequests),
            [targetWindow](bool checked) {
                targetWindow->profiling()->highlightRedrawRequests = checked;
            },
            Label { "Highlight redraw requests" },
          },
          AText::fromItems(
              { "Draws purple rect ", coloredRect(0xff00ff_rgb),
                " over view that requested redraw "
                "(AView::redraw). This can help to find views that causes window to repaint which "
                "affects application performance and device's battery life." }),
          CheckBox {
            AUI_REACT(targetWindow->profiling()->renderToTextureDecay),
            [targetWindow](bool checked) {
                targetWindow->profiling()->renderToTextureDecay = checked;
            },
            Label { "Render to texture decay" },
          },
          AText::fromItems(
              { "Visually displays render to texture optimization by gradually transforming old pixel ",
                "data to gray ", coloredRect(AColor::GRAY),
                " color. When a view is redrawn it pixel "
                "data would be represented as unaffected to gray color and seem bright and saturated "
                "color. From perspective of performance it's good that whole screen transformed to gray "
                "color and thus no redraw is performed." }),
          CheckBox {
            AUI_REACT(targetWindow->profiling()->breakpointOnMarkMinContentSizeInvalid),
            [targetWindow](bool checked) {
                targetWindow->profiling()->breakpointOnMarkMinContentSizeInvalid = checked;
            },
            Label { "Breakpoint on AWindow update layout flag" },
          },
          AText::fromItems(
              { "Stops the attached debugger at the point when window's update layout flag is set. This "
                "can be used to walk through stacktrace and find which view and why triggered layout "
                "update. When breakpoint is triggered, checkbox is unset. Note: when debugger is not attached, "
                "behaviour is undefined." }),
          /// [fromItems]

          header("Scale"),
          Vertical {
            Slider {
              .value = AUI_REACT(*scalingParams),
              .onValueChanged = [scalingParams, targetWindow](aui::float_within_0_1 newValue) {
                  newValue = glm::round(newValue * 6) / 6.f;
                  *scalingParams = newValue;
                  targetWindow->setScalingParams({
                    glm::mix(0.5f, 2.f, float(newValue)),
                  });
              },
            },
            Horizontal {
              Label { "|" },
              SpacerExpanding{},
              Label { "|" },
              SpacerExpanding{},
              Label { "|" },
              SpacerExpanding{},
              Label { "|" },
            },
            Horizontal {
              Label { "50%" },
              SpacerExpanding{},
              Label { "100%" },
              SpacerExpanding{},
              Label { "150%" },
              SpacerExpanding{},
              Label { "200%" },
            },
          },
          AText::fromItems(
              { "In addition to your monitor DPI adjustments, changes scaling factor with AWindow::setScalingParams "
                "API. In this setting, 100% takes no effect." }),
          header("Typography"),
          /// [fromItems]
          CheckBox {
            AUI_REACT(targetWindow->profiling()->showBaseline),
            [targetWindow](bool checked) {
                targetWindow->profiling()->showBaseline = checked;
                targetWindow->redraw();
            },
            Label { "Show baseline" },
          },
          AText::fromItems(
              { "Displays a horizontal line indicating the text baseline. When multiple text views are placed in a row, "
                "their baselines should align for proper visual appearance." }),
        }
        << ".items" AUI_WITH_STYLE {
                      MaxSize { 700_dp, {} },
                    }) });
    setExtraStylesheet(
        { {
            { (c(".items") > t<AView>()) && !t<ALabel>() },
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
          } });
}
