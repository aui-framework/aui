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
#include <AUI/Model/AListModel.h>

#include "AUI/View/AText.h"
#include "Devtools.h"
#include "AUI/View/ADropdownList.h"
#include "AUI/View/ARadioGroup.h"
#include "AUI/View/AGroupBox.h"

using namespace ass;

ViewPropertiesView::ViewPropertiesView(const _<AView>& targetView) {
    setCustomStyle({
      Border {
        1_px,
        0x505050_rgb,
      },
      Padding { 1_px },
      Margin { 4_dp },
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
    AUI_NULLSAFE(targetView->getWindow())->profiling().highlightView = targetView;
    AUI_NULLSAFE(targetView->getWindow())->redraw();

    mTargetView = targetView;
    if (!targetView)
        return;

    ADeque<ass::prop::IPropertyBase*> applicableDeclarations;

    struct ViewModel {
        bool enabled;
        bool expanding;
        Visibility visibility;
    };

    /*
    auto viewModel = _new<ADataBinding<ViewModel>>(ViewModel {
      .enabled = targetView->isEnabled(),
      .expanding = targetView->getExpanding() != glm::ivec2(0),
      .visibility = targetView->getVisibility(),
    });

    targetView&& viewModel(&ViewModel::enabled, &AView::enabledState, &AView::setEnabled);
    targetView&& viewModel(&ViewModel::expanding, aui::select_overload<int>(&AView::setExpanding));
    targetView&& viewModel(&ViewModel::visibility, &AView::visibilityChanged, &AView::setVisibility);

    viewModel->addObserver([this] { requestTargetUpdate(); });

    using namespace declarative;
    auto addressStr = "{}"_format((void*) targetView.get());
    _<AViewContainer> dst = Vertical {
        Horizontal {
          Vertical {
            _new<ALabel>(Devtools::prettyViewName(targetView.get())) with_style { FontSize { 14_pt } },
            Horizontal {
              Label { addressStr },
              Button { "Copy" }.clicked(this, [addressStr] { AClipboard::copyToClipboard(addressStr); }),
            },

            Label { "Min size = {}, {}"_format(targetView->getMinimumWidth(), targetView->getMinimumHeight()) },

            CheckBoxWrapper { Label { "Enabled" } } && viewModel(&ViewModel::enabled),
            AText::fromString((targetView->getAssNames() | ranges::to<AStringVector>()).join(", ")),
            Horizontal {
              Button { "Add \"DevtoolsTest\" stylesheet name" } let {
                      it->setEnabled(!targetView->getAssNames().contains("DevtoolsTest"));
                      connect(it->clicked, [=] {
                          targetView->addAssName("DevtoolsTest");
                          setTargetView(targetView);
                      });
                  },
            },
            CheckBoxWrapper {
              Label { "Expanding" },
            } && viewModel(&ViewModel::expanding),
            GroupBox {
              Label { "Visibility" },
              _new<ARadioGroup>() let {
                      it->setModel(AListModel<AString>::fromVector(
                          AEnumerate<Visibility>::nameToValueMap() | ranges::view::keys | ranges::to_vector));
                      static auto mapping =
                          AEnumerate<Visibility>::nameToValueMap() | ranges::view::values |
                          ranges::to<AVector<Visibility>>;
                      it->setSelectedId(mapping.indexOf((*viewModel)->visibility));
                      connect(it->selectionChanged, [viewModel](AListModelIndex selectionId) {
                          viewModel->setValue(&ViewModel::visibility, mapping[selectionId.getRow()]);
                      });
                  },
            } },
        },

        _new<ALabel>("view's custom style"),
        _new<ALabel>("{") << ".declaration_br",
    };
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

*/
}

void ViewPropertiesView::displayApplicableRule(
    const _<AViewContainer>& dst, ADeque<ass::prop::IPropertyBase*>& applicableDeclarations,
    const ass::PropertyList* rule) {
    for (const auto& decl : rule->declarations()) {
        applicableDeclarations.push_front(decl.get());
        dst->addView(_new<ALabel>(IStringable::toString(decl)) with_style { Opacity { 0.7f } });
    }
    dst->addView(Horizontal {
      _new<ALabel>("},") << ".declaration_br",
    });
    dst->addView(_new<AHDividerView>() with_style {
      BackgroundSolid { 0x505050_rgb }, Margin { 5_dp, 0 }, MinSize { {}, 10_dp } });
}

void ViewPropertiesView::requestTargetUpdate() {
    if (auto targetView = mTargetView.lock()) {
        if (auto targetWindow = targetView->getWindow()) {
            targetWindow->applyGeometryToChildrenIfNecessary();
            targetWindow->redraw();
        }
    }
}
