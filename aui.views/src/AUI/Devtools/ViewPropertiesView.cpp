// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 11/11/2021.
//

#include <AUI/View/ALabel.h>
#include "ViewPropertiesView.h"
#include "Devtools.h"
#include <AUI/ASS/ASS.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Traits/iterators.h>
#include <AUI/Reflect/AReflect.h>
#include <AUI/View/AHDividerView.h>
#include <AUI/Common/IStringable.h>
#include <AUI/View/ACheckBox.h>

using namespace ass;

ViewPropertiesView::ViewPropertiesView(const _<AView>& targetView) {
    setCustomStyle({ Border {
                           1_px,
                           0x505050_rgb,
                   },
                   Padding { 1_px },
                   Margin { 4_dp } });
    setTargetView(targetView);
}

void ViewPropertiesView::setTargetView(const _<AView>& targetView) {
    mTargetView = targetView;
    if (!targetView) return;

    ADeque<ass::decl::IDeclarationBase*> applicableDeclarations;

    using namespace declarative;
    auto dst = Vertical {
            _new<ALabel>(Devtools::prettyViewName(targetView.get())) with_style { FontSize {14_pt } },
            CheckBoxWrapper {
                Label { "Enabled "},
            } let {
                it->setChecked(targetView->isEnabled());
                connect(it->checked, [this](bool v) {
                    if (auto s = mTargetView.lock()) s->setEnabled(v);
                    requestTargetUpdate();
                });
            },

            CheckBoxWrapper {
                Label {"Expanding"},
            } let {
                it->setChecked(targetView->getExpanding() != glm::ivec2(0));
                connect(it->checked, [this](bool v) {
                    if (auto s = mTargetView.lock()) s->setExpanding(v);
                    requestTargetUpdate();
                });
            },

            _new<ALabel>("view's custom style"),
            _new<ALabel>("{") << ".declaration_br",

    };

    displayApplicableRule(dst, applicableDeclarations, &targetView->getCustomAss());

    for (auto r : aui::reverse_iterator_wrap(targetView->getAssHelper()->getPossiblyApplicableRules())) {
        if (r->getSelector().isStateApplicable(targetView.get())) {
            AStringVector sl;
            for (auto& ss : r->getSelector().getSubSelectors()) {
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
            displayApplicableRule(dst, applicableDeclarations, r);
        }
    }
    dst->addView( _new<ALabel>("}") << ".declaration_br");
    AScrollArea::setContents(dst);

    updateLayout();
    redraw();
}

void ViewPropertiesView::displayApplicableRule(const _<AViewContainer>& dst,
                                               ADeque<ass::decl::IDeclarationBase*>& applicableDeclarations,
                                               const RuleWithoutSelector* rule) {

    for (const auto& decl : rule->getDeclarations()) {
        applicableDeclarations.push_front(decl.get());
        dst->addView(_new<ALabel>("<internal {}>"_format(AReflect::name(decl.get()))) with_style{ Opacity {0.7f } });
    }
    dst->addView(Horizontal {
            _new<ALabel>("},") << ".declaration_br",
    });
    dst->addView(_new<AHDividerView>() with_style{BackgroundSolid {0x505050_rgb }, Margin {5_dp, 0 }, MinSize {{}, 10_dp } });
}

void ViewPropertiesView::requestTargetUpdate() {
    if (auto targetView = mTargetView.lock()) {
        if (auto targetWindow = targetView->getWindow()) {
            targetWindow->updateLayout();
            targetWindow->redraw();
        }
    }
}
