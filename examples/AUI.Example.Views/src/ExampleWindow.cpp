// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

    setLayout(_new<AVerticalLayout>());
    AStylesheet::global().addRules({
         {
             c(".all_views_wrap") > t<AViewContainer>(),
             Padding { 16_dp },
         }
    });

    addView(Horizontal {
        _new<ADrawableView>(IDrawable::fromUrl(":img/logo.svg")) with_style { FixedSize { 32_dp } },
        AText::fromString("Building beautiful programs in pure C++ without chromium embedded framework") with_style { Expanding{} },
    });
    addView(Horizontal {
            _new<ADrawableView>(IDrawable::fromUrl(":img/gf.gif")) with_style { FixedSize { 100_dp } },
            AText::fromString("Add animated gif images!") with_style { Expanding{} },
    });
    _<ATabView> tabView;
    addView(tabView = _new<ATabView>() let {
        it->addTab(AScrollArea::Builder().withContents(std::conditional_t<aui::platform::current::is_mobile(), Vertical, Horizontal> {
            Vertical {
                // buttons
                GroupBox {
                    Label { "Buttons" },
                    Vertical {
                        _new<AButton>("Common button"),
                        _new<AButton>("Default button") let { it->setDefault(); },
                        _new<AButton>("Disabled button") let { it->setDisabled(); },
                    },
                },

                // checkboxes
                GroupBox {
                    CheckBoxWrapper { Label { "Checkboxes" } } let { it->setChecked(true); },
                    Vertical {
                        CheckBoxWrapper { Label { "Unchecked checkbox" } },
                        CheckBoxWrapper { Label { "Selected checkbox" } } let { it->setChecked(true); },
                        CheckBoxWrapper { Label { "Disabled checkbox" } } let { it->setDisabled(); },
                    },
                },

                // radiobuttons
                GroupBox {
                    Label { "Radiobuttons" },
                    RadioGroup {
                        RadioButton { "Radiobutton 1" } let { it->setChecked(true); },
                        RadioButton { "Radiobutton 2" },
                        RadioButton { "Radiobutton 3" },
                        RadioButton { "Disabled radiobutton" } let { it->disable(); },
                    },
                },

                // comboboxes
                GroupBox {
                    Label  { "Comboboxes" },
                    Vertical {
                        _new<ADropdownList>(AListModel<AString>::make({
                                                                          "Combobox 1",
                                                                          "Combobox 2",
                                                                          "Combobox 3",
                                                                          "Combobox 4",
                                                                          "Combobox 5",
                                                                          "Combobox 6",
                                                                  })),
                        _new<ADropdownList>(AListModel<AString>::make({"Disabled combobox"})) let { it->setDisabled(); },
                    },
                },
                GroupBox {
                    Label { "Drag area" },

                    _new<ADragArea>() let {
                        it with_style {
                                MinSize { 100_dp },
                                Border { 1_px, 0x0_rgb, },
                        };
                        it->addView(ADragArea::convertToDraggable(_new<AButton>("Drag me!"), false));
                    },
                } with_style { Expanding{} },
            },
            Vertical {
                GroupBox {
                    Label { "Window factory" },
                    Vertical {
                        CheckBoxWrapper { Label { "Resizeable" } },
                        _new<AButton>("Show window").connect(&AButton::clicked, this, [&] {
                            auto w = _new<ACustomWindow>("Custom window without caption", 400_dp, 300_dp);
                            fillWindow(w);
                            w->show();
                            w->setWindowStyle(WindowStyle::MODAL);
                            mWindows << w;
                        }),
                        _new<AButton>("Close all windows").connect(&AButton::clicked, this, [&] {
                            for (auto& w : mWindows)
                                w->close();
                            mWindows.clear();
                        }),
                    }
                },

                GroupBox {
                    Label { "System dialog" },
                    Vertical{
                        _new<AButton>("Show file chooser").connect(&AView::clicked, this, [&] {
                            ADesktop::browseForFile(this).onSuccess([&](const APath& f) {
                                if (f.empty()) {
                                    AMessageBox::show(this, "Result", "Cancelled");
                                } else {
                                    AMessageBox::show(this, "Result", "File: {}"_format(f));
                                }
                            });
                        }),
                        _new<AButton>("Show folder chooser").connect(&AView::clicked, this, [&] {
                            ADesktop::browseForDir(this).onSuccess([&](const APath& f) {
                                if (f.empty()) {
                                    AMessageBox::show(this, "Result", "Cancelled");
                                } else {
                                    AMessageBox::show(this, "Result", "Folder: {}"_format(f));
                                }
                            });
                        }),
                        _new<AButton>("Message box").connect(&AView::clicked, this, [&] {
                            AMessageBox::show(this, "Title", "Message", AMessageBox::Icon::INFO);
                        }),
                        _new<AButton>("Cause assertion fail").connect(&AView::clicked, this, [&] {
                            assert(("assertion fail", false));
                        }),
                    }
                },

                // list view
                GroupBox {
                    Label { "List view" },
                    [] { // lambda style inlining
                        auto model = _new<DemoListModel>();

                        return Vertical {
                                Horizontal {
                                        _new<AButton>("Add").connect(&AButton::clicked, slot(model)::addItem),
                                        _new<AButton>("Remove").connect(&AButton::clicked, slot(model)::removeItem),
                                        _new<ASpacerExpanding>(),
                                },
                                _new<AListView>(model)
                        };
                    }(),
                },

                // foreach
                GroupBox {
                    Label { "AForEachUI" },
                    [this] { // lambda style inlining
                        auto model = _new<AListModel<MyModel>>();

                        return Vertical {
                                Horizontal {
                                        _new<AButton>("Add").connect(&AButton::clicked, this, [model] {
                                            static std::default_random_engine re;
                                            do_once {
                                            re.seed(std::time(nullptr));
                                        };
                                            static std::uniform_real_distribution<float> d(0.f, 1.f);
                                            model->push_back({ AColor(d(re), d(re), d(re), 1.f) });
                                        }),
                                        _new<AButton>("Remove").connect(&AButton::clicked, this, [model] {
                                            if (!model->empty()) {
                                                model->pop_back();
                                            }
                                        }),
                                        _new<ASpacerExpanding>(),
                                },
                                ui_for (i, model, AWordWrappingLayout) {
                                    return Horizontal {
                                            _new<ALabel>(i.color.toString()) with_style {
                                                    TextColor { i.color.readableBlackOrWhite() },
                                            }
                                    } with_style {
                                                   BackgroundSolid { i.color },
                                                   BorderRadius { 6_pt },
                                                   Margin { 2_dp, 4_dp },
                                           };
                                }
                        };
                    }(),
                },

                // tree view
                GroupBox {
                    Label { "Tree view" },
                    [] { // lambda style inlining
                        auto model = _new<DemoTreeModel>();

                        return _new<ATreeView>(model);
                    }(),
                },
            },
            Vertical::Expanding {
                // fields
                GroupBox {
                    Label { "Fields" },
                    Vertical::Expanding {
                        Label { "Text field" },
                        _new<ATextField>() let { it->focus(); },
                        Label { "Number picker" },
                        _new<ANumberPicker>(),
                        _new<ADoubleNumberPicker>(),
                        Label { "Text area" },
                        _new<ATextArea>("Copyright (c) 2021 Alex2772\n\n"
                                        "Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated "
                                        "documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the "
                                        "rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to "
                                        "permit persons to whom the Software is furnished to do so, subject to the following conditions:\n\n"

                                        "The above copyright notice and this permission notice shall be included in all copies or substantial portions of the "
                                        "Software.\n\n"
                                        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE "
                                        "WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR "
                                        "COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR "
                                        "OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.) ") let { it->setExpanding(); },
                    }
                } with_style { Expanding{} }
            }
        }), "Common");

        it->addTab(Vertical {
            _new<ALabel>("Horizontal splitter"),
            ASplitter::Horizontal().withItems({_new<AButton>("One"),
                                               _new<AButton>("Two"),
                                               _new<AButton>("Three"),
                                               _new<AButton>("Four"),
                                               _new<AButton>("Five")}),
            _new<ALabel>("Vertical splitter"),
            ASplitter::Vertical().withItems({_new<AButton>("One"),
                                             _new<AButton>("Two"),
                                             _new<AButton>("Three"),
                                             _new<AButton>("Four"),
                                             _new<AButton>("Five")}).build() let { it->setExpanding(); },
            _new<ALabel>("Grid splitter"),
            AGridSplitter::Builder().withItems(AVector<AVector<_<AView>>>::generate(5, [](size_t y) {
               return AVector<_<AView>>::generate(5, [&](size_t x) {
                   return _new<AButton>("{}x{}"_format(x, y));
               });
            }))
        } let { it->setExpanding(); }, "Splitters");

        it->addTab(AScrollArea::Builder().withContents(Vertical {
            ASplitter::Horizontal().withItems({
                Vertical::Expanding {
                    _new<ALabel>("Default"),
                    AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                      "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                      "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                      "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                      "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                      "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style { TextAlign::JUSTIFY },
                },
                Vertical::Expanding {
                    _new<ALabel>("Word breaking"),
                    AText::fromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                                      "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                                      "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                                      "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                                      "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                                      "proident, sunt in culpa qui officia deserunt mollit anim id est laborum",
                                      { WordBreak::BREAK_ALL })
                },
            }),
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
        } let { it->setExpanding(); }), "Text");

        it->addTab(Vertical {
            // Rulers
            _new<ALabel>("ARulerArea"),
            _new<ARulerArea>(_new<AView>() with_style { MinSize { 100_dp, 100_dp },
                                                        BackgroundGradient { 0x0_rgb, 0x404040_rgb, ALayoutDirection::VERTICAL },
                                                        MaxSize { {}, 300_dp },
                                                        Expanding{}, }) with_style { Expanding{} },

            _new<DemoGraphView>(),
            _new<ADragNDropView>(),

            Label { "Custom cursor" } with_style {
                ACursor{ ":img/logo.svg", 64 },
            }

        } let { it->setExpanding(); }, "Others");

        it->setExpanding();
    });

    addView(Horizontal{
        Centered {
            _new<AButton>("Show all views...").connect(&AButton::clicked, this, [] {
                _new<AllViewsWindow>()->show();
            })
        },
        _new<ASpacerExpanding>(),
        _new<ASpinner>(),
        CheckBoxWrapper {
            Label { "Enabled" },
        } let {
            it->setChecked();
            connect(it->checked, slot(tabView)::setEnabled);
        },
        _new<ALabel>("\u00a9 Alex2772, 2021, alex2772.ru") let {
            it << "#copyright";
            it->setEnabled(false);
        }
    });
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
