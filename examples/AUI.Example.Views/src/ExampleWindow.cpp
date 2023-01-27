// AUI Framework - Declarative UI toolkit for modern C++17
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

#include <AUI/View/ARadioButton.h>
#include <AUI/View/ARadioGroup.h>
#include <AUI/Model/AListModel.h>
#include "ExampleWindow.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/View/AButton.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Platform/ACustomCaptionWindow.h"
#include "AUI/View/ALabel.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/ANumberPicker.h"
#include "AUI/View/ADoubleNumberPicker.h"
#include "AUI/View/ASpacerExpanding.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "DemoListModel.h"
#include "DemoTreeModel.h"
#include "AUI/View/ASpinner.h"
#include "DemoGraphView.h"
#include "AUI/View/AGroupBox.h"
#include "AUI/View/ADragNDropView.h"
#include "AUI/Util/ALayoutInflater.h"
#include <AUI/Model/AListModel.h>
#include <AUI/View/ADropdownList.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/ASS/ASS.h>
#include <AUI/View/ASelectableLabel.h>
#include <AUI/View/AListView.h>
#include <AUI/View/ATextArea.h>
#include <AUI/View/ARulerView.h>
#include <AUI/View/AForEachUI.h>
#include <AUI/View/ARulerArea.h>
#include <AUI/View/ATreeView.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/View/ADragArea.h>
#include <random>
#include <AUI/View/ASplitter.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/ATabView.h>
#include <AUI/View/AGridSplitter.h>
#include <AUI/View/AText.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/Traits/platform.h>

using namespace declarative;

struct MyModel {
    AColor color;
};

void fillWindow(_<AViewContainer> t)
{
    t->setLayout(_new<AStackedLayout>());
    t->addView(_new<ALabel>("Window contents"));
}


class AllViewsWindow: public AWindow {
public:
    AllViewsWindow(): AWindow("All views", 300_dp, 500_dp, nullptr, WindowStyle::MODAL) {
        setContents(Centered {
            AScrollArea::Builder().withContents(Vertical {
                Centered { _new<ALabel>("ALabel") },
                Centered { _new<AButton>("AButton") },
                Centered { _new<ATextField>() let { it->setText("ATextField"); } },
            } << ".all_views_wrap").withExpanding()
        });
    }
};

ExampleWindow::ExampleWindow(): AWindow("Examples", 800_dp, 700_dp)
{
    allowDragNDrop();

    setLayout(_new<AHorizontalLayout>());
    AStylesheet::global().addRules({
         {
             c(".all_views_wrap") > t<AViewContainer>(),
             Padding { 16_dp },
         }
    });

        addView (
                AScrollArea::Builder().withContents(Horizontal {
                        ASplitter::Horizontal().withItems({
                                                                  Horizontal::Expanding {
                                                                          _new<ALabel>("Default"),
                                                                          AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                                                                            "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                                                                            "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                                                                            "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                                                                            "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                                                                            "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style { TextAlign::JUSTIFY },
                                                                  },
                                                          }),
                        Horizontal {
                                _new<ADrawableView>(IDrawable::fromUrl(":img/gf.gif")) with_style { FixedSize { 100_dp } }, // gif from https://tenor.com/view/cat-gif-26024730
                                AText::fromString("Add animated gif images!") with_style { Expanding{} },
                        },
                        [] {
                            _<AViewContainer> v1 = Vertical {};
                            _<AViewContainer> v2 = Vertical {};
                            for (int i = 0; i <= 9; ++i) {
                                v1->addView(Horizontal {
                                        _new<ALabel>("{} px"_format(i + 6)),
                                        _new<ALabel>("Hello! [] .~123`") with_style { FontSize { AMetric(i + 6, AMetric::T_PX) } }
                                });
                                v2->addView(Horizontal {
                                        _new<ALabel>("{} px"_format(i + 16)),
                                        _new<ALabel>("Hello! [] .~123`") with_style { FontSize { AMetric(i + 16, AMetric::T_PX) } }
                                });
                            }
                            return Horizontal { v1, v2 };
                        }(),
                } let { it->setExpanding(); }).build() with_style { ScrollbarAppearance { ScrollbarAppearance::VISIBLE } }
        );
}

void ExampleWindow::onDragDrop(const ADragNDrop::DropEvent& event) {
    ABaseWindow::onDragDrop(event);

    for (const auto&[k, v] : event.data.data()) {
        ALogger::info("Drop") << "[" << k << "] = " << AString::fromUtf8(v);
    }

    auto surface = createOverlappingSurface({0, 0}, {100, 100}, false);
    _<AViewContainer> popup = Vertical {
        Label { "Drop event" } with_style { FontSize { 18_pt }, TextAlign::CENTER, },
        [&]() -> _<AView> {
            if (auto u = event.data.urls()) {
                auto url = u->first();
                if (auto icon = ADesktop::iconOfFile(url.path())) {
                    return Centered { _new<ADrawableView>(icon) with_style { FixedSize { 64_dp } } };
                }
            }
            return nullptr;
        }(),
        AText::fromString("Caught drop event. See the logger output for contents.") with_style { TextAlign::CENTER, MinSize { 100_dp, 40_dp } },
        Centered { Button { "OK" }.clicked(this, [surface] {
            surface->close();
        }) }
    };
    ALayoutInflater::inflate(surface, popup);
    popup->pack();

    surface->setOverlappingSurfaceSize(popup->getSize());
    surface->setOverlappingSurfacePosition((getSize() - popup->getSize()) / 2);
}

bool ExampleWindow::onDragEnter(const ADragNDrop::EnterEvent& event) {
    return true;
}
