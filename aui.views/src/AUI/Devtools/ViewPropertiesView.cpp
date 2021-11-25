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
    setCustomAss({ Border {
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
    getContentContainer()->setLayout(_new<AVerticalLayout>());
    getContentContainer()->addView(_new<ALabel>(Devtools::prettyViewName(targetView.get())) with_style { FontSize {14_pt } });

    ADeque<ass::decl::IDeclarationBase*> applicableDeclarations;


    this->getContentContainer()->addView(Vertical {
            _new<ACheckBox>("Enabled") let {
                it->setChecked(targetView->isEnabled());
                connect(it->checked, [this](bool v) {
                    if (auto s = mTargetView.lock()) s->setEnabled(v);
                    requestTargetUpdate();
                });
            },
    });

    this->getContentContainer()->addView(Vertical {
            _new<ACheckBox>("Expanding") let {
                it->setChecked(targetView->getExpanding() != glm::ivec2(0));
                connect(it->checked, [this](bool v) {
                    if (auto s = mTargetView.lock()) s->setExpanding(v);
                    requestTargetUpdate();
                });
            },
    });

    // element style
    this->getContentContainer()->addView(Horizontal {
            _new<ALabel>("view's custom style"),
            _new<ALabel>("{") << ".declaration_br",
    });

    displayApplicableRule(applicableDeclarations, &targetView->getCustomAss());

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
                this->getContentContainer()->addView(_new<ALabel>(sl.join(',') + ","));
            } else {
                this->getContentContainer()->addView(_new<ALabel>("{" + sl.join(',') + "},"));
            }
            displayApplicableRule(applicableDeclarations, r);
        }
    }
    updateLayout();
    redraw();
}

void ViewPropertiesView::displayApplicableRule(ADeque<ass::decl::IDeclarationBase*>& applicableDeclarations,
                                       const RuleWithoutSelector* rule) {

    for (auto& decl : rule->getDeclarations()) {
        applicableDeclarations.push_front(decl);
        this->getContentContainer()->addView(_new<ALabel>("<internal {}>"_format(AReflect::name(decl))) with_style{Opacity {0.7f } });
    }
    this->getContentContainer()->addView(Horizontal {
            _new<ALabel>("},") << ".declaration_br",
    });
    this->getContentContainer()->addView(_new<AHDividerView>() with_style{BackgroundSolid {0x505050_rgb }, Margin {5_dp, 0 }, MinSize {{}, 10_dp } });
}

void ViewPropertiesView::requestTargetUpdate() {
    if (auto targetView = mTargetView.lock()) {
        if (auto targetWindow = targetView->getWindow()) {
            targetWindow->updateLayout();
            targetWindow->redraw();
        }
    }
}
