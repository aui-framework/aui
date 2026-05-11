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
// Created by Alex2772 on 11/11/2021.
//

#include <range/v3/all.hpp>

#include "ViewPropertiesView.h"
#include <AUI/ASS/ASS.h>
#include <AUI/Common/IStringable.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/AClipboard.h>
#include <AUI/Reflect/AReflect.h>
#include <AUI/Traits/iterators.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ACheckBox.h>
#include <AUI/View/AHDividerView.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AButton.h>
#include <AUI/Model/AListModel.h>

#include "AUI/View/AText.h"
#include "Devtools.h"
#include "AUI/View/ADropdownList.h"
#include "AUI/View/ARadioGroup.h"
#include "AUI/View/AGroupBox.h"
#include "AUI/Common/AString.h"
#include "AUI/View/AForEachUI.h"
#include <chrono>

using namespace ass;
using namespace declarative;
using namespace std::chrono;

namespace {
void logDurationIfNonZero(const char* phase, high_resolution_clock::time_point start) {
    const auto durationMs = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
    if (durationMs == 0) {
        return;
    }
    ALogger::info("ViewPropertiesView") << phase << " took " << durationMs << " ms";
}
}

ViewPropertiesView::ViewPropertiesView(const _<AView>& targetView) {
    setCustomStyle({
      Border {
        1_px,
        0x505050_rgb,
      },
      Padding { 1_px },
      MinSize { 200_dp },
      ScrollbarAppearance(ScrollbarAppearance::ALWAYS, ScrollbarAppearance::ON_DEMAND),
      Expanding(),
    });
    setTargetView(targetView);
}

glm::ivec2 ViewPropertiesView::onIntrinsicMeasure(AConstraints constraints) {
    const auto start = high_resolution_clock::now();
    auto result = AScrollArea::onIntrinsicMeasure(constraints);
    logDurationIfNonZero(("measure " + std::to_string(constraints.maxInline) + " " + std::to_string(constraints.maxBlock)).c_str(), start);
    return result;
}

AMinMaxAxis ViewPropertiesView::onComputeIntrinsicMinMaxAxis(int height) {
    const auto start = high_resolution_clock::now();
    auto result = AScrollArea::onComputeIntrinsicMinMaxAxis(height);
    logDurationIfNonZero(("min max " + std::to_string(height)).c_str(), start);
    return result;
}

void ViewPropertiesView::onLayout(int w, int h) {
    const auto start = high_resolution_clock::now();
    AScrollArea::onLayout(w, h);
    logDurationIfNonZero("layout", start);
}

void ViewPropertiesView::render(ARenderContext context) {
    const auto start = high_resolution_clock::now();
    AScrollArea::render(context);
    logDurationIfNonZero("render", start);
}

void ViewPropertiesView::requestLayout() {
  AScrollArea::requestLayout();
}

void ViewPropertiesView::setTargetView(const _<AView>& targetView) {
    if (!targetView) {
        return;
    }
    AUI_NULLSAFE(targetView->getWindow())->profiling()->highlightView = targetView;
    AUI_NULLSAFE(targetView->getWindow())->redraw();

    mTargetView = targetView;
    if (!targetView)
        return;

    ADeque<ass::prop::IPropertyBase*> applicableDeclarations;

    auto addressStr = "{}"_format((void*) targetView.get());
    _<AViewContainer> dst = Vertical {
        _new<ALabel>(Devtools::prettyViewName(targetView.get())) AUI_OVERRIDE_STYLE { FontSize { 14_pt } },
        Horizontal {
          Label { addressStr },
          Button { Label { "Copy" }, [addressStr] { AClipboard::copyToClipboard(addressStr); } },
        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },

        Label { AUI_REACT("Min size = {}px"_format(targetView->getMinSize())) },
        Label { AUI_REACT("Max size = {}px"_format(targetView->getMaxSize())) },
        Label { AUI_REACT("Size = {}px"_format(targetView->size())) },
        Label { AUI_REACT("Compute min size = {}px"_format(targetView->computeMinMaxAxis().min)) },
        Label { AUI_REACT("Compute max size = {}px"_format(targetView->computeMinMaxAxis().max)) },
        Label { AUI_REACT("Measure (width -1) = {}px"_format(targetView->measure(AConstraints {}))) },
        Label { AUI_REACT("Measure (width 0) = {}px"_format(targetView->measure(AConstraints::fixedInline(0)))) },

        Vertical {
          CheckBox {
            AUI_REACT(targetView->enabled()),
            [targetView](bool enabled) { targetView->enabled() = enabled; },
            Label { "Enabled" },
          },
          CheckBox {
            AUI_REACT(targetView->expanding() != glm::ivec2(0)),

            [this, targetView](bool expanding) { targetView->expanding() = expanding ? glm::ivec2(1) : glm::ivec2(0); },
            Label { "Expanding" },
          },
        },

        AText::fromString((targetView->getAssNames() | ranges::to<AStringVector>()).join(", "), { WordBreak::BREAK_ALL }),

        Button {
          Label { "Add \"DevtoolsTest\" stylesheet name" },

          [this, targetView] {
              setTargetView(targetView);
              targetView->addAssName("DevtoolsTest");
          },
        } AUI_LET { it->setEnabled(!targetView->getAssNames().contains("DevtoolsTest")); },

        GroupBox {
          Label { "Visibility" },
          AUI_DECLARATIVE_FOR(i, aui::enumerate::ALL_VALUES<Visibility>, AVerticalLayout) {
              return RadioButton {
                  AUI_REACT(targetView->visibility() == i),
                  [=] { targetView->visibility() = i;  },
                  Label { "{}"_format(i) },
              };
          },
        },

        Label { "view's custom style" },
        Label { "{" } << ".declaration_br",
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp }, Padding { 4_dp } };

    displayApplicableRule(dst, applicableDeclarations, &targetView->getCustomAss());

    for (const auto& r : aui::reverse_iterator_wrap(targetView->getAssHelper()->getPossiblyApplicableRules())) {
        if (r.getSelector().isStateApplicable(targetView.get())) {
            AStringVector sl;
            for (auto& ss : r.getSelector().getSubSelectors()) {
                if (auto classOf = _cast<class_of>(ss)) {
                    sl << "ass::class_of(\"{}\")"_format(classOf->getClasses().join(", "));
                } else {
                    sl << IStringable::toString(ss);
                }
            }
            if (sl.size() == 1) {
                dst->addView(_new<ALabel>(sl.join(',') + ","));
            } else {
                dst->addView(_new<ALabel>("{" + sl.join(',') + "},"));
            }
            displayApplicableRule(dst, applicableDeclarations, &r);
        }
    }
    dst->addView(_new<ALabel>("}") << ".declaration_br");
    AScrollArea::setContents(dst);

    redraw();
}

void ViewPropertiesView::displayApplicableRule(
    const _<AViewContainer>& dst, ADeque<ass::prop::IPropertyBase*>& applicableDeclarations,
    const ass::PropertyList* rule) {
    const auto start = high_resolution_clock::now();
    for (const auto& decl : rule->declarations()) {
        applicableDeclarations.push_front(decl.get());
        dst->addView(_new<ALabel>(IStringable::toString(decl)) AUI_OVERRIDE_STYLE { Opacity { 0.7f } });
    }
    dst->addView(Horizontal {
      _new<ALabel>("},") << ".declaration_br",
    });
    dst->addView(
        _new<AHDividerView>()
            AUI_OVERRIDE_STYLE { BackgroundSolid { 0x505050_rgb }, Margin { 5_dp, 0 }, MinSize { {}, 10_dp } });
    logDurationIfNonZero("displayApplicableRule", start);
}

void ViewPropertiesView::requestTargetUpdate() {
    const auto start = high_resolution_clock::now();
    if (auto targetView = mTargetView.lock()) {
        if (auto targetWindow = targetView->getWindow()) {
            targetWindow->redraw();
        }
    }
    logDurationIfNonZero("requestTargetUpdate", start);
}
