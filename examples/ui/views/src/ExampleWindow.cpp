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

#include <range/v3/all.hpp>
#include <AUI/View/ARadioButton.h>
#include <AUI/View/ARadioGroup.h>
#include <AUI/Model/AListModel.h>
#include <AUI/Audio/ASS/Property/Sound.h>
#include "AUI/ASS/Property/BackgroundSolid.h"
#include "AUI/ASS/Property/Border.h"
#include "AUI/ASS/Property/TransformOffset.h"
#include "AUI/ASS/Selector/on_state.h"
#include "ExampleWindow.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/Model/ATreeModel.h"
#include "AUI/Util/AMetric.h"
#include "AUI/View/A2FingerTransformArea.h"
#include "AUI/View/AButton.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Platform/ACustomCaptionWindow.h"
#include "AUI/View/ACircleProgressBar.h"
#include "AUI/View/ALabel.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/AProgressBar.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/ANumberPicker.h"
#include "AUI/View/ADoubleNumberPicker.h"
#include "AUI/View/ASpacerExpanding.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "DemoListModel.h"
#include "AUI/View/ASpinnerV2.h"
#include "DemoGraphView.h"

#include "AUI/Audio/IAudioPlayer.h"

#include "AUI/View/AGroupBox.h"
#include "AUI/View/ADragNDropView.h"
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/View/ASlider.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/IO/AByteBufferInputStream.h"
#include "AUI/Curl/ACurl.h"
#include "AUI/View/ASpinnerV2.h"
#include "AUI/View/ASpacerFixed.h"
#include <AUI/Model/AListModel.h>
#include <AUI/View/ADropdownList.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/ASS/ASS.h>
#include <AUI/View/AListView.h>
#include <AUI/View/ATextArea.h>
#include <AUI/View/ARulerView.h>
#include <AUI/View/AForEachUI.h>
#include <AUI/View/ARulerArea.h>
#include <AUI/View/ATreeView.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/View/ADragArea.h>
#include <memory>
#include <random>
#include <AUI/View/ASplitter.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/ATabView.h>
#include <AUI/View/AGridSplitter.h>
#include <AUI/View/AText.h>
#include <AUI/View/ADrawableView.h>
#include <AUI/Traits/platform.h>

using namespace declarative;

void fillWindow(_<AViewContainer> t) {
    t->setLayout(std::make_unique<AStackedLayout>());
    t->addView(_new<ALabel>("Window contents"));
}

class AllViewsWindow : public AWindow {
public:
    AllViewsWindow() : AWindow("All views", 300_dp, 500_dp, nullptr, WindowStyle::MODAL) {
        setContents(Centered {
          AScrollArea::Builder()
              .withContents(
                  Vertical {
                    Centered { _new<ALabel>("ALabel") },
                    Centered { _new<AButton>("AButton") },
                    Centered { _new<ATextField>() AUI_LET { it->setText("ATextField"); } },
                  }
                  << ".all_views_wrap")
              .withExpanding() });
    }
};

/// [label_coloring]
static _<AView> link(const AString& url) {
    return Label { url } AUI_WITH_STYLE {
               TextColor { AColor::BLUE },
               BorderBottom { 1_px, AColor::BLUE },
               ACursor::POINTER,
           } AUI_LET { AObject::connect(it->clicked, AObject::GENERIC_OBSERVER, [url] { APlatform::openUrl(url); }); };
}
/// [label_coloring]

static _<AView> checkBoxesDemo() {
    struct State {
        AProperty<bool> checked1 = false, checked2 = true;
    };
    auto state = _new<State>();
    return Vertical {
        CheckBox {
          AUI_REACT(state->checked1),
          [state](bool checked) { state->checked1 = checked; },
          Label { "Unchecked checkbox" },
        },
        CheckBox {
          AUI_REACT(state->checked2),
          [state](bool checked) { state->checked2 = checked; },
          Label { "Checked checkbox" },
        },
        CheckBox {
          AUI_REACT(false),
          Label { "Disabled checkbox" },
        } AUI_LET { it->disable(); },
    };
}

static _<AView> radioButtonsDemo() {
    auto selection = _new<AProperty<int>>(0);
    return Vertical {
        RadioButton {
          AUI_REACT(*selection == 0),
          [selection] { *selection = 0; },
          Label { "Radiobutton 1" },
        },
        RadioButton {
          AUI_REACT(*selection == 1),
          [selection] { *selection = 1; },
          Label { "Radiobutton 2" },
        },
        RadioButton {
          AUI_REACT(*selection == 2),
          [selection] { *selection = 2; },
          Label { "Radiobutton 3" },
        },
        RadioButton {
          AUI_REACT(*selection == 3),
          [selection] { *selection = 3; },
          Label { "Radiobutton 4" },
        } AUI_LET { it->disable(); },
    };
}

ExampleWindow::ExampleWindow() : AWindow("Examples", 800_dp, 700_dp) {
    allowDragNDrop();

    setLayout(std::make_unique<AVerticalLayout>());
    AStylesheet::global().addRules({ {
      c(".all_views_wrap") > t<AViewContainer>(),
      Padding { 16_dp },
    } });
#if AUI_PLATFORM_IOS || AUI_PLATFORM_ANDROID
    setCustomStyle({
      Padding { 64_dp, {}, 16_dp },
    });
#endif

    setCustomStyle({
      LayoutSpacing { 4_dp },
    });

    addView(Horizontal {
      _new<ADrawableView>(IDrawable::fromUrl(":img/logo.svg")) AUI_WITH_STYLE { FixedSize { 32_dp } },
      AText::fromString("Building beautiful programs in pure C++ without chromium embedded framework") AUI_WITH_STYLE {
            Expanding(1, 0),
          },
      Horizontal {} AUI_LET {
              mAsync << AUI_THREADPOOL {
                  auto drawable = IDrawable::fromUrl(
                      "https://raster.shields.io/github/stars/aui-framework/aui?style=raster&logo=github");
                  AUI_UI_THREAD {
                      auto view = Icon { drawable } AUI_WITH_STYLE {
                          FixedSize { 80_dp, 20_dp },
                          BackgroundImage { {}, {}, {}, Sizing::COVER },
                          Margin { 4_dp },
                          ACursor::POINTER,
                      };
                      connect(view->clicked, [] {
                          APlatform::openUrl("https://github.com/aui-framework/aui/stargazers");
                      });
                      it->addView(view);
                  };
              };
          },
    });

    _<ATabView> tabView;
    _<AProgressBar> progressBar = _new<AProgressBar>();
    _<ACircleProgressBar> circleProgressBar = _new<ACircleProgressBar>();

    addView(tabView = _new<ATabView>() AUI_LET {
        it->addTab(
            AScrollArea::Builder().withContents(std::conditional_t<
                                                aui::platform::current::is_mobile(), Vertical, Horizontal> {
              Vertical {
                // buttons
                GroupBox {
                  Label { "Buttons" },
                  Vertical {
                    Button {
                      Label { "Common button" },
                      [this] { AMessageBox::show(this, "Title", "Message"); },
                    },
                    Button {
                      Label { "Default button" },
                      [this] { AMessageBox::show(this, "Title", "Message"); },
                      true,
                    },
                    Button { Label { "Disabled button" } } AUI_LET { it->setDisabled(); },
                    Button {
                          Horizontal {
                            Icon { ":img/logo.svg" },
                            SpacerFixed { 2_dp },
                            Label { "Button with icon" },
                          },
                      [this] {
                        AMessageBox::show(this, "Title", "Message");
                      },
                    },
                  } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
                },

                // checkboxes
                GroupBox {
                  Label { "Checkboxes" } ,
                  checkBoxesDemo(),
                },

                // radiobuttons
                GroupBox {
                  Label { "Radiobuttons" },
                  radioButtonsDemo(),
                },

                // dropdown lists
                GroupBox {
                  Label { "Dropdown lists" },
                  Vertical {
                    _new<ADropdownList>(AListModel<AString>::make({
                      "Dropdown list 1",
                      "Dropdown list 2",
                      "Dropdown list 3",
                      "Dropdown list 4",
                      "Dropdown list 5",
                      "Dropdown list 6",
                    })),
                    _new<ADropdownList>(AListModel<AString>::make({ "Disabled dropdown" })) AUI_LET { it->setDisabled(); },
                  } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
                },
                GroupBox {
                  Label { "Drag area" },

                  _new<ADragArea>() AUI_LET {
                          it AUI_WITH_STYLE {
                              MinSize { 100_dp },
                              Border { 1_px, 0x0_rgb },
                          };
                          it->addView(ADragArea::convertToDraggable(_new<AButton>("Drag me!"), false));
                      },
                } AUI_WITH_STYLE { Expanding {} },
              } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
              Vertical {
#if AUI_PLATFORM_WIN || AUI_PLATFORM_LINUX || AUI_PLATFORM_MACOS
                GroupBox {
                  Label { "Window factory" },
                  Vertical {
                    // CheckBoxWrapper { Label { "Resizeable" } }, TODO
                    _new<AButton>("Show window")
                        .connect(&AButton::clicked, this,
                                 [&] {
                                     auto w = _new<AWindow>("Basic window", 400_dp, 300_dp, this, WindowStyle::MODAL);
                                     fillWindow(w);
                                     w->show();
                                     mWindows << w;
                                 }),
                    _new<AButton>("Show window without caption")
                        .connect(&AButton::clicked, this,
                                 [&] {
                                     auto w = _new<ACustomWindow>("Custom window without caption", 400_dp, 300_dp);
                                     fillWindow(w);
                                     w->show();
                                     mWindows << w;
                                 }),
                    _new<AButton>("Show window custom caption")
                        .connect(&AButton::clicked, this,
                                 [&] {
                                     auto w =
                                         _new<ACustomCaptionWindow>("Custom window custom caption", 400_dp, 300_dp);
                                     fillWindow(w->getContentContainer());
                                     w->show();
                                     w->setWindowStyle(WindowStyle::MODAL);
                                     mWindows << w;
                                 }),
                    _new<AButton>("Close all windows")
                        .connect(&AButton::clicked, this,
                                 [&] {
                                     for (auto& w : mWindows) w->close();
                                     mWindows.clear();
                                 }),
                  } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
                },
#endif

                GroupBox {
                  Label { "System dialog" },
                  Vertical {
                    _new<AButton>("Show file chooser")
                        .connect(&AView::clicked, this,
                                 [&] {
                                     mAsync << ADesktop::browseForFile(this).onSuccess([&](const APath& f) {
                                         if (f.empty()) {
                                             AMessageBox::show(this, "Result", "Cancelled");
                                         } else {
                                             AMessageBox::show(this, "Result", "File: {}"_format(f));
                                         }
                                     });
                                 }),
                    _new<AButton>("Show folder chooser")
                        .connect(&AView::clicked, this,
                                 [&] {
                                     mAsync << ADesktop::browseForDir(this).onSuccess([&](const APath& f) {
                                         if (f.empty()) {
                                             AMessageBox::show(this, "Result", "Cancelled");
                                         } else {
                                             AMessageBox::show(this, "Result", "Folder: {}"_format(f));
                                         }
                                     });
                                 }),
                    _new<AButton>("Message box")
                        .connect(&AView::clicked, this,
                                 [&] {
                                     /// [AMessageBox]
                                     auto response = AMessageBox::show(this,
                                                       "Title",
                                                       "Message",
                                                       AMessageBox::Icon::NONE,
                                                       AMessageBox::Button::OK);
                                     ALogger::info("Example") << "Response: " << int(response);
                                     /// [AMessageBox]
                                 }),
                    _new<AButton>("Cause assertion fail")
                        .connect(&AView::clicked, this, [&] { AUI_ASSERT_NO_CONDITION("assertion fail"); }),
                    _new<AButton>("Cause hang")
                        .connect(&AView::clicked, this,
                                 [&] {
                                     for (;;)
                                         ;
                                 }),
                    _new<AButton>("Cause access violation")
                        .connect(&AView::clicked, this,
                                 [&] {
                                     try {
                                         *((volatile int*) nullptr) = 123;
                                     } catch (const AException& e) {
                                         ALogger::info("Example") << "Successfully caught access violation: " << e;
                                     }
                                 }),
                  } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
                },

                // list view
                GroupBox {
                  Label { "List view" },
                  [] {   // lambda style inlining
                      auto model = _new<DemoListModel>();

                      return Vertical {
                          Horizontal {
                            _new<AButton>("Add").connect(&AButton::clicked, AUI_SLOT(model)::addItem),
                            _new<AButton>("Remove").connect(&AButton::clicked, AUI_SLOT(model)::removeItem),
                          } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
                          _new<AListView>(model)
                      } AUI_WITH_STYLE { LayoutSpacing { 4_dp } };
                  }(),
                },

                // foreach
                GroupBox {
                  Label { "AForEachUI" },
                  [this] {   // lambda style inlining
                      struct State {
                          AProperty<AVector<AColor>> colors;
                      };
                      auto state = _new<State>();

                      return Vertical {
                          Horizontal {
                            _new<AButton>("Add").connect(
                                &AButton::clicked, this,
                                [state] {
                                    static std::default_random_engine re;
                                    AUI_DO_ONCE { re.seed(std::time(nullptr)); };
                                    static std::uniform_real_distribution<float> d(0.f, 1.f);
                                    state->colors.writeScope()->push_back({ AColor(d(re), d(re), d(re), 1.f) });
                                }),
                            _new<AButton>("Remove").connect(
                                &AButton::clicked, this,
                                [state] {
                                    if (!state->colors->empty()) {
                                        state->colors.writeScope()->pop_back();
                                    }
                                }),
                            _new<ASpacerExpanding>(),
                          } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
                          AUI_DECLARATIVE_FOR(i, *state->colors, AWordWrappingLayout) {
                              return Horizontal {
                                  _new<ALabel>(i.toString()) AUI_WITH_STYLE {
                                      TextColor { i.readableBlackOrWhite() },
                                  }
                              } AUI_WITH_STYLE {
                                  BackgroundSolid { i },
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
                  [] {   // lambda style inlining
                      return _new<ATreeView>(_new<ATreeModel<AString>>(AVector<ATreeModel<AString>::Item>{
                              {
                                "dir1",
                                {
                                  { "file1" },
                                  { "file2" },
                                }
                              },
                              {
                                "dir2",
                                {
                                  { "file3" },
                                  { "file4" },
                                }
                              },
                            }));
                  }(),
                } AUI_WITH_STYLE { Expanding {} },
              } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
              Vertical::Expanding {
                // fields
                GroupBox {
                  Label { "Progressbar" },
                  Vertical {
                    progressBar,
                    circleProgressBar,
                    GroupBox {
                      Label { "Slider" },
                      Vertical {
                        _new<ASlider>()
                            .connect(&ASlider::valueChanging, AUI_SLOT(progressBar)::setValue)
                            .connect(&ASlider::valueChanging, AUI_SLOT(circleProgressBar)::setValue),
                      },
                    },
                  } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
                },
                GroupBox {
                  Label { "Scaling factor" },
                  Horizontal {
                    _new<ANumberPicker>().connect(&ANumberPicker::valueChanged,
                                                  [](int64_t x) {
                                                      AWindow::current()->setScalingParams(
                                                          { x * 0.25f,
                                                            std::nullopt });
                                                  }) AUI_LET {
                            it->setMin(1);
                            it->setMax(12);
                            it->setValue(4);
                        },
                    Label { "x0.25" } } },
                GroupBox {
                  Label { "Fields" },
                  Vertical::Expanding {
                    Label { "Text field" },
                    _new<ATextField>() AUI_LET { it->focus(); },
                    Label { "Number picker" },
                    _new<ANumberPicker>(),
                    _new<ADoubleNumberPicker>(),
                    Label { "Text area" },
                    AScrollArea::Builder()
                            .withContents(_new<ATextArea>(
                                "AUI Framework - Declarative UI toolkit for modern C++20\n"
                                "Copyright (C) 2020-2025 Alex2772 and Contributors\n"
                                "\n"
                                "SPDX-License-Identifier: MPL-2.0\n"
                                "\n"
                                "This Source Code Form is subject to the terms of the Mozilla "
                                "Public License, v. 2.0. If a copy of the MPL was not distributed with this "
                                "file, You can obtain one at http://mozilla.org/MPL/2.0/."))
                            .build()
                        << ".input-field" AUI_LET { it->setExpanding(); },
                  } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
                } AUI_WITH_STYLE { Expanding {} },
              } AUI_WITH_STYLE { LayoutSpacing { 4_dp } }
            } AUI_WITH_STYLE { LayoutSpacing { 4_dp } }),
            "Common");

#if !AUI_PLATFORM_EMSCRIPTEN
        mWavAudio = IAudioPlayer::fromUrl(":sound/sound1.wav");
        mOggAudio = IAudioPlayer::fromUrl(":sound/sound1.ogg");

        it->addTab(
            AScrollArea::Builder().withContents(std::conditional_t<
                                                aui::platform::current::is_mobile(), Vertical, Horizontal> {
              Horizontal {
                Vertical {
                  _new<ALabel>("Play music using AUI!"),
                  _new<AButton>("Play .wav music").connect(&AButton::clicked, AUI_SLOT(mWavAudio)::play),
                  _new<AButton>("Stop .wav music").connect(&AButton::clicked, AUI_SLOT(mWavAudio)::stop),
                  _new<AButton>("Pause .wav music").connect(&AButton::clicked, AUI_SLOT(mWavAudio)::pause),
                  _new<ALabel>("Volume control"),
                  _new<ASlider>().connect(
                      &ASlider::valueChanging, this,
                      [player = mWavAudio](aui::float_within_0_1 value) {
                          player->setVolume(static_cast<uint32_t>(float(value) * 256.f));
                      }),
                } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
                Vertical {
                  _new<ALabel>("Play music using AUI!"),
                  _new<AButton>("Play .ogg music").connect(&AButton::clicked, AUI_SLOT(mOggAudio)::play),
                  _new<AButton>("Stop .ogg music").connect(&AButton::clicked, AUI_SLOT(mOggAudio)::stop),
                  _new<AButton>("Pause .ogg music").connect(&AButton::clicked, AUI_SLOT(mOggAudio)::pause),
                  _new<ALabel>("Volume control"),
                  _new<ASlider>().connect(
                      &ASlider::valueChanging, this,
                      [player = mOggAudio](aui::float_within_0_1 value) {
                          player->setVolume(static_cast<uint32_t>(float(value) * 256.f));
                      }),
                } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
                Vertical {
                  _new<AButton>("Button produces sound when clicked") AUI_WITH_STYLE {
                        ass::on_state::Activated {
                          ass::Sound { IAudioPlayer::fromUrl(":sound/click.ogg") },
                        },
                      },
                } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
              } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
            }),
            "Sounds");
#endif

        it->addTab(
            AScrollArea::Builder().withContents(
                std::conditional_t<aui::platform::current::is_mobile(), Vertical, Horizontal> { Horizontal {
                  Vertical {
                    _new<ALabel>("Gif support!"),
                    _new<ADrawableView>(IDrawable::fromUrl(":img/gf.gif")) AUI_WITH_STYLE { FixedSize { 100_dp } },   // gif from https://tenor.com/view/cat-gif-26024730
                  },
                  Vertical {
                    _new<ALabel>("Animated WebP support!"),
                    _new<ADrawableView>(AUrl(":img/anim.webp")) AUI_WITH_STYLE { FixedSize { 320_px, 240_px } } } } }),
            "Images");

        it->addTab(
            Vertical {
              _new<ALabel>("Horizontal splitter"),
              ASplitter::Horizontal().withItems({
                _new<AButton>("One"),
                _new<AButton>("Two"),
                _new<AButton>("Three"),
                _new<AButton>("Four"),
                _new<AButton>("Five"),
              }).build() AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
              ASplitter::Horizontal().withItems({
                _new<AButton>("One"),
                _new<AButton>("Two"),
                _new<AButton>("Three"),
                SpacerExpanding(),
                _new<AButton>("Four"),
                _new<AButton>("Five"),
              }).build() AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
              _new<ALabel>("Vertical splitter"),
              ASplitter::Vertical()
                      .withItems({ _new<AButton>("One"), _new<AButton>("Two"), _new<AButton>("Three"),
                                   _new<AButton>("Four"), _new<AButton>("Five") })
                      .build() AUI_WITH_STYLE { LayoutSpacing { 4_dp }, Expanding{} },
              _new<ALabel>("Grid splitter"),
              AGridSplitter::Builder()
                      .withItems(AVector<AVector<_<AView>>>::generate(
                          5,
                          [](size_t y) {
                              return AVector<_<AView>>::generate(5, [&](size_t x) {
                                  return _new<AButton>("{}x{}"_format(x, y));
                              });
                          }))
                      .build() AUI_WITH_STYLE { LayoutSpacing { 4_dp }, Expanding{} },
            } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
            "Splitters");

        it->addTab(
            AScrollArea::Builder().withContents(
                Vertical {
                  ASplitter::Horizontal().withItems({
                    Vertical::Expanding {
                      _new<ALabel>("Default"),
                      AText::fromString(
                          "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") AUI_WITH_STYLE { ATextAlign::JUSTIFY },
                    } AUI_WITH_STYLE { MinSize { 200_dp } },
                    Vertical::Expanding {
                      _new<ALabel>("Word breaking"),
                      AText::fromString(
                          "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                          "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
                          "quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
                          "consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
                          "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non "
                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum",
                          { WordBreak::BREAK_ALL }),
                    } AUI_WITH_STYLE { MinSize { 200_dp } },
                  }).build() AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
                  [] {
                      _<AViewContainer> v1 = Vertical {};
                      _<AViewContainer> v2 = Vertical {};
                      for (int i = 0; i <= 9; ++i) {
                          v1->addView(Horizontal {
                            _new<ALabel>("{} px"_format(i + 6)),
                            _new<ALabel>("Hello! [] .~123`") AUI_WITH_STYLE { FontSize { AMetric(i + 6, AMetric::T_PX) } } });
                          v2->addView(Horizontal {
                            _new<ALabel>("{} px"_format(i + 16)),
                            _new<ALabel>("Hello! [] .~123`") AUI_WITH_STYLE { FontSize { AMetric(i + 16, AMetric::T_PX) } } });
                      }
                      return Horizontal { v1, v2 };
                  }(),
                } AUI_LET { it->setExpanding(); }),
            "Text");

        it->addTab(
            Vertical {
              _new<A2FingerTransformArea>() AUI_LET {
                      it->setCustomStyle({
                        MinSize { 256_dp },
                        Border { 1_px, AColor::BLACK },
                      });

                      _<AView> blackRect = Stacked {
                          Stacked { _new<AButton>("Hi") } AUI_WITH_STYLE {
                            FixedSize { 200_dp, 100_dp },
                            BackgroundSolid { AColor::BLACK },
                            TextColor { AColor::WHITE },
                            ATextAlign::CENTER,
                          }
                      };
                      ALayoutInflater::inflate(it, Stacked { blackRect });
                      connect(
                          it->transformed, blackRect,
                          [blackRect = blackRect.get(),
                           keptTransform = _new<A2DTransform>()](const A2DTransform& transform) {
                              keptTransform->applyDelta(transform);
                              blackRect->setCustomStyle({
                                TransformOffset {
                                  AMetric(keptTransform->offset.x, AMetric::T_PX),
                                  AMetric(keptTransform->offset.y, AMetric::T_PX) },
                                TransformScale { keptTransform->scale },
                                TransformRotate { keptTransform->rotation },
                              });
                          });
                  },
              _new<ADragNDropView>(),

              Horizontal {
                Centered {
                  Vertical {
                    Label { "Custom cursor" } AUI_WITH_STYLE {
                          ACursor { ":img/logo.svg", 64 },
                        },
                    link("https://github.com/aui-framework/aui"),
                  },
                },
                Stacked {
                  Icon { ":img/logo.svg" } AUI_WITH_STYLE { FixedSize(32_dp) },
                  Centered {
                    Label { "Blur" } AUI_WITH_STYLE { Margin { 1_dp, 16_dp } },
                  } AUI_WITH_STYLE {
                        Expanding(1, 0),
                        Backdrop { Backdrop::GaussianBlur { 9_dp } },
                        BackgroundSolid { AColor::WHITE.transparentize(0.5f) },
                      },
                },
              },
            } AUI_LET { it->setExpanding(); },
            "Others");

        it->setExpanding();
    });

    addView(Horizontal {
      Centered {
        _new<AButton>("Show all views...").connect(&AButton::clicked, this, [] { _new<AllViewsWindow>()->show(); }),
      },
      SpacerExpanding{},
      Centered { _new<ASpinnerV2>() },
      CheckBox {
        AUI_REACT(tabView->enabled()),
        [tabView](bool checked) { tabView->enabled() = checked; },
        Label { "Enabled" },
      },
      _new<ALabel>("\u00a9 Alex2772, 2025, alex2772.ru") AUI_LET {
              it << "#copyright";
              it->setEnabled(false);
          },
    } AUI_WITH_STYLE { LayoutSpacing { 4_dp } });
}

void ExampleWindow::onDragDrop(const ADragNDrop::DropEvent& event) {
    AWindowBase::onDragDrop(event);

    for (const auto& [k, v] : event.data.data()) {
        ALogger::info("Drop") << "[" << k << "] = " << AString::fromUtf8(v);
    }

    auto surface = createOverlappingSurface({ 0, 0 }, { 100, 100 }, false);
    _<AViewContainer> popup = Vertical {
        Label { "Drop event" } AUI_WITH_STYLE {
              FontSize { 18_pt },
              ATextAlign::CENTER,
            },
        [&]() -> _<AView> {
            if (auto u = event.data.urls()) {
                auto url = u->first();
                if (auto icon = ADesktop::iconOfFile(url.path())) {
                    return Centered { _new<ADrawableView>(icon) AUI_WITH_STYLE { FixedSize { 64_dp } } };
                }
            }
            return nullptr;
        }(),
        AText::fromString("Caught drop event. See the logger output for contents.") AUI_WITH_STYLE { ATextAlign::CENTER, MinSize { 100_dp, 40_dp } },
        Centered { Button { Label { "OK" }, [surface] { surface->close(); } } },
    };
    ALayoutInflater::inflate(surface, popup);
    popup->pack();

    surface->setOverlappingSurfaceSize(popup->size());
    surface->setOverlappingSurfacePosition((size() - *popup->size()) / 2);
}

bool ExampleWindow::onDragEnter(const ADragNDrop::EnterEvent& event) { return true; }
