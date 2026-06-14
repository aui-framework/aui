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

using namespace ass;
using namespace declarative;

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

        Label { AUI_REACT("Min size = {}px"_format(targetView->getMinimumSize())) },
        Label { AUI_REACT("Size = {}px"_format(targetView->size())) },

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

        AText::fromString((targetView->getAssNames() | ranges::to<AStringVector>()).join(", ")),

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

    applyGeometryToChildrenIfNecessary();
    redraw();
}

void ViewPropertiesView::displayApplicableRule(
    const _<AViewContainer>& dst, ADeque<ass::prop::IPropertyBase*>& applicableDeclarations,
    const ass::PropertyList* rule) {
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
}

void ViewPropertiesView::requestTargetUpdate() {
    if (auto targetView = mTargetView.lock()) {
        if (auto targetWindow = targetView->getWindow()) {
            targetWindow->applyGeometryToChildrenIfNecessary();
            targetWindow->redraw();
        }
    }
}
