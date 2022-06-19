/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
#include "AUI/View/ASpacer.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "DemoListModel.h"
#include "DemoTreeModel.h"
#include "AUI/View/ASpinner.h"
#include "DemoGraphView.h"
#include <AUI/Model/AListModel.h>
#include <AUI/View/AComboBox.h>
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

using namespace ass;

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
    setLayout(_new<AVerticalLayout>());
    AStylesheet::inst().addRules({
         {
             c(".all_views_wrap") > t<AViewContainer>(),
             Padding { 16_dp },
         }
    });

    addView(Horizontal {
        _new<ADrawableView>(IDrawable::fromUrl(":img/logo.svg")) with_style { FixedSize { 32_dp } },
        AText::fromString("Building beautiful programs in pure C++ without chromium embedded framework") with_style { Expanding{} },
    });
    _<ATabView> tabView;
    addView(tabView = _new<ATabView>() let {
        it->addTab(AScrollArea::Builder().withContents(std::conditional_t<aui::platform::current::is_mobile(), Vertical, Horizontal> {
            Vertical {
                // buttons
                _new<ALabel>("Buttons"),
                _new<AButton>("Common button"),
                _new<AButton>("Default button") let { it->setDefault(); },
                _new<AButton>("Disabled button") let { it->setDisabled(); },

                // checkboxes
                _new<ALabel>("Checkboxes"),
                _new<ACheckBox>("Unchecked checkbox"),
                _new<ACheckBox>("Selected checkbox") let { it->setChecked(true); },
                _new<ACheckBox>("Disabled checkbox") let { it->setDisabled(); },

                // radiobuttons
                _new<ALabel>("Radiobuttons"),
                _new<ARadioGroup>(AListModel<AString>::make({"Radiobutton 1",
                                                                  "Radiobutton 2",
                                                                  "Radiobutton 3",
                                                                  "Disabled radiobutton"})) let {
                    it->getViews()[3]->setDisabled();
                },

                // comboboxes
                _new<ALabel>("Comboboxes"),
                _new<AComboBox>(AListModel<AString>::make({
                                                                  "Combobox 1",
                                                                  "Combobox 2",
                                                                  "Combobox 3",
                                                                  "Combobox 4",
                                                                  "Combobox 5",
                                                                  "Combobox 6",
                                                          })),
                _new<AComboBox>(AListModel<AString>::make({"Disabled combobox"})) let { it->setDisabled(); },
                _new<ADragArea>() let {
                    it with_style {
                            MinSize { 100_dp },
                            Border { 1_px, 0x0_rgb, },
                    };
                    it->addView(ADragArea::convertToDraggable(_new<AButton>("Drag me!"), false));
                },
            },
            Vertical {
                _new<ALabel>("Windows"),
                _new<AButton>("Common window").connect(&AButton::clicked, this, [&] {
                    auto w = _new<AWindow>("Common window", 400_dp, 300_dp);
                    fillWindow(w);
                    w->show();
                    mWindows << w;
                }),
                _new<AButton>("Dialog window").connect(&AButton::clicked, this, [&] {
                    auto w = _new<AWindow>("Dialog window", 400_dp, 300_dp);
                    fillWindow(w);
                    w->show();
                    w->setWindowStyle(WindowStyle::MODAL);
                    mWindows << w;
                }),
                _new<AButton>("Modal window").connect(&AButton::clicked, this, [&] {
                    auto w = _new<AWindow>("Modal window", 400_dp, 300_dp, this, WindowStyle::MODAL);
                    fillWindow(w);
                    w->show();
                    mWindows << w;
                }),
                _new<AButton>("Custom window with caption").connect(&AButton::clicked, this, [&] {
                    auto w = _new<ACustomCaptionWindow>("Custom window with caption", 400_dp, 300_dp);
                    w->getCaptionContainer()->setLayout(_new<AStackedLayout>());
                    w->getCaptionContainer()->addView(_new<AButton>("Custom button here"));
                    fillWindow(w->getContentContainer());
                    w->show();
                    //w->setWindowStyle(WindowStyle::DIALOG);
                    mWindows << w;
                }),
                _new<AButton>("Custom window without caption").connect(&AButton::clicked, this, [&] {
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

                _new<ALabel>("System dialog"),
                _new<AButton>("Show file chooser").connect(&AView::clicked, this, [&] {
                    ADesktop::browseForFile().onSuccess([&](const APath& f) {
                        if (f.empty()) {
                            AMessageBox::show(this, "Result", "Cancelled");
                        } else {
                            AMessageBox::show(this, "Result", "File: {}"_format(f));
                        }
                    });
                }),
                _new<AButton>("Show folder chooser").connect(&AView::clicked, this, [&] {
                    ADesktop::browseForDir().onSuccess([&](const APath& f) {
                        if (f.empty()) {
                            AMessageBox::show(this, "Result", "Cancelled");
                        } else {
                            AMessageBox::show(this, "Result", "Folder: {}"_format(f));
                        }
                    });
                }),
                // list view
                _new<ALabel>("List view"),
                [] { // lambda style inlining
                    auto model = _new<DemoListModel>();

                    return Vertical {
                            Horizontal {
                                    _new<AButton>("Add").connect(&AButton::clicked, slot(model)::addItem),
                                    _new<AButton>("Remove").connect(&AButton::clicked, slot(model)::removeItem),
                                    _new<ASpacer>(),
                            },
                            _new<AListView>(model)
                    };
                }(),

                    // foreach
                _new<ALabel>("AForEachUI"),
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
                                    _new<ASpacer>(),
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
                // tree view
                _new<ALabel>("Tree view"),
                [] { // lambda style inlining
                    auto model = _new<DemoTreeModel>();

                    return _new<ATreeView>(model);
                }()
            },
            Vertical::Expanding {
                // fields
                _new<ALabel>("Fields"),
                _new<ALabel>("Text field"),
                _new<ATextField>() let { it->focus(); },
                _new<ALabel>("Number picker"),
                _new<ANumberPicker>(),
                _new<ALabel>("Text area"),
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
                                                        BackgroundGradient { 0x0_rgb, 0x404040_rgb, LayoutDirection::VERTICAL },
                                                        MaxSize { {}, 300_dp },
                                                        Expanding{}, }) with_style { Expanding{} },

            _new<DemoGraphView>(),

        } let { it->setExpanding(); }, "Others");

        it->setExpanding();
    });

    addView(Horizontal{
        _new<AButton>("Show all views...").connect(&AButton::clicked, this, [] {
            _new<AllViewsWindow>()->show();
        }),
        _new<ASpacer>(),
        _new<ASpinner>(),
        _new<ACheckBox>("Enabled") let {
            it->setChecked();
            connect(it->checked, slot(tabView)::setEnabled);
        },
        _new<ALabel>("\u00a9 Alex2772, 2021, alex2772.ru") let {
            it << "#copyright";
            it->setEnabled(false);
        }
    });
}
