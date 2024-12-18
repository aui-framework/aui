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
#include "AUI/View/ASpinner.h"
#include "DemoGraphView.h"

#include "AUI/Audio/IAudioPlayer.h"

#include "AUI/View/AGroupBox.h"
#include "AUI/View/ADragNDropView.h"
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/View/ASlider.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/IO/AByteBufferInputStream.h"
#include "AUI/Curl/ACurl.h"
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

struct MyModel {
    AColor color;
};

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
                    Centered { _new<ATextField>() let { it->setText("ATextField"); } },
                  }
                  << ".all_views_wrap")
              .withExpanding() });
    }
};

ExampleWindow::ExampleWindow() : AWindow("Examples", 800_dp, 700_dp) {
    allowDragNDrop();

    setLayout(std::make_unique<AVerticalLayout>());
    AStylesheet::global().addRules({ {
      c(".all_views_wrap") > t<AViewContainer>(),
      Padding { 16_dp },
    } });

    addView(Horizontal {
      _new<ADrawableView>(IDrawable::fromUrl(":img/logo.svg")) with_style { FixedSize { 32_dp } },
      AText::fromString("Building beautiful programs in pure C++ without chromium embedded framework") with_style {
        Expanding(1, 0),
      },
      Horizontal {} let {
              mAsync << async {
                  auto drawable = IDrawable::fromUrl("https://raster.shields.io/github/stars/aui-framework/aui?style=raster&logo=github");
                  ui_thread {
                      auto view = Icon { drawable } with_style {
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

    addView(tabView = _new<ATabView>() let {
        it->addTab(
            AScrollArea::Builder().withContents(std::conditional_t<
                                                aui::platform::current::is_mobile(), Vertical, Horizontal> {
              Vertical {
                // buttons
                GroupBox {
                  Label { "Buttons" },
                  Vertical {
                    _new<AButton>("Common button"),
                    _new<AButton>("Default button") let { it->setDefault(); },
                    _new<AButton>("Disabled button") let { it->setDisabled(); },
                    Button {
                      Icon { ":img/logo.svg" },
                      Label { "Button with icon" },
                    }
                        .clicked(this, [&] { AMessageBox::show(this, "Title", "Message"); }),
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
                  Label { "Comboboxes" },
                  Vertical {
                    _new<ADropdownList>(AListModel<AString>::make({
                      "Combobox 1",
                      "Combobox 2",
                      "Combobox 3",
                      "Combobox 4",
                      "Combobox 5",
                      "Combobox 6",
                    })),
                    _new<ADropdownList>(AListModel<AString>::make({ "Disabled combobox" })) let { it->setDisabled(); },
                  },
                },
                GroupBox {
                  Label { "Drag area" },

                  _new<ADragArea>() let {
                          it with_style {
                              MinSize { 100_dp },
                              Border { 1_px, 0x0_rgb },
                          };
                          it->addView(ADragArea::convertToDraggable(_new<AButton>("Drag me!"), false));
                      },
                } with_style { Expanding {} },
              },
              Vertical {
#if AUI_PLATFORM_WIN || AUI_PLATFORM_LINUX || AUI_PLATFORM_MACOS
                GroupBox {
                  Label { "Window factory" },
                  Vertical {
                    // CheckBoxWrapper { Label { "Resizeable" } }, TODO
                    _new<AButton>("Show window")
                        .connect(&AButton::clicked, this,
                                 [&] {
                                     auto w = _new<AWindow>("Basic window", 400_dp, 300_dp);
                                     fillWindow(w);
                                     w->show();
                                     w->setWindowStyle(WindowStyle::MODAL);
                                     mWindows << w;
                                 }),
                    _new<AButton>("Show window without caption")
                        .connect(&AButton::clicked, this,
                                 [&] {
                                     auto w = _new<ACustomWindow>("Custom window without caption", 400_dp, 300_dp);
                                     fillWindow(w);
                                     w->show();
                                     w->setWindowStyle(WindowStyle::MODAL);
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
                  } },
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
                                 [&] { AMessageBox::show(this, "Title", "Message", AMessageBox::Icon::INFO); }),
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
                                         *((int*) 0) = 123;
                                     } catch (const AException& e) {
                                         ALogger::info("Example") << "Successfully caught access violation: " << e;
                                     }
                                 }),
                  },
                },

                // list view
                GroupBox {
                  Label { "List view" },
                  [] {   // lambda style inlining
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
                  [this] {   // lambda style inlining
                      auto model = _new<AListModel<MyModel>>();

                      return Vertical {
                          Horizontal {
                            _new<AButton>("Add").connect(
                                &AButton::clicked, this,
                                [model] {
                                    static std::default_random_engine re;
                                    do_once { re.seed(std::time(nullptr)); };
                                    static std::uniform_real_distribution<float> d(0.f, 1.f);
                                    model->push_back({ AColor(d(re), d(re), d(re), 1.f) });
                                }),
                            _new<AButton>("Remove").connect(
                                &AButton::clicked, this,
                                [model] {
                                    if (!model->empty()) {
                                        model->pop_back();
                                    }
                                }),
                            _new<ASpacerExpanding>(),
                          },
                          AUI_DECLARATIVE_FOR(i, model, AWordWrappingLayout) {
                              return Horizontal {
                                  _new<ALabel>(i.color.toString()) with_style {
                                      TextColor { i.color.readableBlackOrWhite() },
                                  }
                              }
                              with_style {
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
                  [] {   // lambda style inlining
                      return _new<ATreeView>(_new<ATreeModel<AString>>(AVector<ATreeModel<AString>::Item>{
                              {
                                .value = "dir1",
                                .children = {
                                  { .value = "file1" },
                                  { .value = "file2" },
                                }
                              },
                              {
                                .value = "dir2",
                                .children = {
                                  { .value = "file3" },
                                  { .value = "file4" },
                                }
                              },
                            }));
                  }(),
                },
              },
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
                            .connect(&ASlider::valueChanging, slot(progressBar)::setValue)
                            .connect(&ASlider::valueChanging, slot(circleProgressBar)::setValue),
                      } },
                  },
                },
                GroupBox {
                  Label { "Scaling factor" },
                  Horizontal {
                    _new<ANumberPicker>().connect(&ANumberPicker::valueChanged,
                                                  [](int64_t x) {
                                                      AWindow::current()->setScalingParams(
                                                          { .scalingFactor = x * 0.25f,
                                                            .minimalWindowSizeDp = std::nullopt });
                                                  }) let {
                            it->setMin(1);
                            it->setMax(12);
                            it->setValue(4);
                        },
                    Label { "x0.25" } } },
                GroupBox {
                  Label { "Fields" },
                  Vertical::Expanding {
                    Label { "Text field" },
                    _new<ATextField>() let { it->focus(); },
                    Label { "Number picker" },
                    _new<ANumberPicker>(),
                    _new<ADoubleNumberPicker>(),
                    Label { "Text area" },
                    AScrollArea::Builder()
                            .withContents(_new<ATextArea>(
                                "AUI Framework - Declarative UI toolkit for modern C++20\n"
                                "Copyright (C) 2020-2024 Alex2772 and Contributors\n"
                                "\n"
                                "SPDX-License-Identifier: MPL-2.0\n"
                                "\n"
                                "This Source Code Form is subject to the terms of the Mozilla "
                                "Public License, v. 2.0. If a copy of the MPL was not distributed with this "
                                "file, You can obtain one at http://mozilla.org/MPL/2.0/."))
                            .build()
                        << ".input-field" let { it->setExpanding(); },
                  } } with_style { Expanding {} } } }),
            "Common");

#if !AUI_PLATFORM_EMSCRIPTEN
        mWavAudio = IAudioPlayer::fromUrl(":sound/sound1.wav");
        mOggAudio = IAudioPlayer::fromUrl(":sound/sound1.ogg");

        it->addTab(
            AScrollArea::Builder().withContents(std::conditional_t<aui::platform::current::is_mobile(), Vertical,
                                                                   Horizontal> { Horizontal {
              Vertical {
                _new<ALabel>("Play music using AUI!"),
                _new<AButton>("Play .wav music").connect(&AButton::clicked, slot(mWavAudio)::play),
                _new<AButton>("Stop .wav music").connect(&AButton::clicked, slot(mWavAudio)::stop),
                _new<AButton>("Pause .wav music").connect(&AButton::clicked, slot(mWavAudio)::pause),
                _new<ALabel>("Volume control"),
                _new<ASlider>()
                    .connect(&ASlider::valueChanging, this,
                             [player = mWavAudio](aui::float_within_0_1 value) {
                                 player->setVolume(static_cast<uint32_t>(float(value) * 256.f));
                             }) },
              Vertical {
                _new<ALabel>("Play music using AUI!"),
                _new<AButton>("Play .ogg music").connect(&AButton::clicked, slot(mOggAudio)::play),
                _new<AButton>("Stop .ogg music").connect(&AButton::clicked, slot(mOggAudio)::stop),
                _new<AButton>("Pause .ogg music").connect(&AButton::clicked, slot(mOggAudio)::pause),
                _new<ALabel>("Volume control"),
                _new<ASlider>()
                    .connect(&ASlider::valueChanging, this,
                             [player = mOggAudio](aui::float_within_0_1 value) {
                                 player->setVolume(static_cast<uint32_t>(float(value) * 256.f));
                             }) },
              Vertical { _new<AButton>("Button produces sound when clicked") with_style { ass::on_state::Activated {
                ass::Sound { IAudioPlayer::fromUrl(":sound/click.ogg") },
              } } } } }),
            "Sounds");
#endif

        it->addTab(
            AScrollArea::Builder().withContents(
                std::conditional_t<aui::platform::current::is_mobile(), Vertical, Horizontal> { Horizontal {
                  Vertical {
                    _new<ALabel>("Gif support!"),
                    _new<ADrawableView>(IDrawable::fromUrl(":img/gf.gif")) with_style {
                      FixedSize { 100_dp } },   // gif from https://tenor.com/view/cat-gif-26024730
                  },
                  Vertical {
                    _new<ALabel>("Animated WebP support!"),
                    _new<ADrawableView>(AUrl(":img/anim.webp")) with_style { FixedSize { 320_px, 240_px } } } } }),
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
              }),
              ASplitter::Horizontal().withItems({
                _new<AButton>("One"),
                _new<AButton>("Two"),
                _new<AButton>("Three"),
                SpacerExpanding(),
                _new<AButton>("Four"),
                _new<AButton>("Five"),
              }),
              _new<ALabel>("Vertical splitter"),
              ASplitter::Vertical()
                      .withItems({ _new<AButton>("One"), _new<AButton>("Two"), _new<AButton>("Three"),
                                   _new<AButton>("Four"), _new<AButton>("Five") })
                      .build() let { it->setExpanding(); },
              _new<ALabel>("Grid splitter"),
              AGridSplitter::Builder()
                      .withItems(AVector<AVector<_<AView>>>::generate(
                          5,
                          [](size_t y) {
                              return AVector<_<AView>>::generate(5, [&](size_t x) {
                                  return _new<AButton>("{}x{}"_format(x, y));
                              });
                          }))
                      .build() let { it->setExpanding(); },
            },
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
                          "proident, sunt in culpa qui officia deserunt mollit anim id est laborum") with_style {
                        ATextAlign::JUSTIFY },
                    } with_style { MinSize { 200_dp } },
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
                    } with_style { MinSize { 200_dp } },
                  }),
                  [] {
                      _<AViewContainer> v1 = Vertical {};
                      _<AViewContainer> v2 = Vertical {};
                      for (int i = 0; i <= 9; ++i) {
                          v1->addView(Horizontal {
                            _new<ALabel>("{} px"_format(i + 6)),
                            _new<ALabel>("Hello! [] .~123`") with_style {
                              FontSize { AMetric(i + 6, AMetric::T_PX) } } });
                          v2->addView(Horizontal {
                            _new<ALabel>("{} px"_format(i + 16)),
                            _new<ALabel>("Hello! [] .~123`") with_style {
                              FontSize { AMetric(i + 16, AMetric::T_PX) } } });
                      }
                      return Horizontal { v1, v2 };
                  }(),
                } let { it->setExpanding(); }),
            "Text");

        it->addTab(
            Vertical {
                _new<A2FingerTransformArea>() let {
                        it->setCustomStyle({
                          MinSize { 256_dp },
                          Border { 1_px, AColor::BLACK },
                        });

                        _<AView> blackRect = Stacked { Stacked { _new<AButton>("Hi") } with_style {
                          FixedSize { 200_dp, 100_dp },
                          BackgroundSolid { AColor::BLACK },
                          TextColor { AColor::WHITE },
                          ATextAlign::CENTER,
                        } };
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

                    Label { "Custom cursor" } with_style {
                        ACursor { ":img/logo.svg", 64 },
                    },
                    Horizontal {
                    Label { "github.com/aui-framework/aui" }.clicked(
                        this, [] { APlatform::openUrl("https://github.com/aui-framework/aui"); }) with_style {
                        TextColor { AColor::BLUE },
                        BorderBottom { 1_px, AColor::BLUE },
                        ACursor::POINTER,
                    },
                }
            } let { it->setExpanding(); },
            "Others");

        it->setExpanding();
    });

    addView(Horizontal {
      Centered {
        _new<AButton>("Show all views...").connect(&AButton::clicked, this, [] { _new<AllViewsWindow>()->show(); }) },
      _new<ASpacerExpanding>(), _new<ASpinner>(),
      CheckBoxWrapper {
        Label { "Enabled" },
      } let {
              it->setChecked();
              connect(it->checked, slot(tabView)::setEnabled);
          },
      _new<ALabel>("\u00a9 Alex2772, 2021, alex2772.ru") let {
              it << "#copyright";
              it->setEnabled(false);
          } });
}

void ExampleWindow::onDragDrop(const ADragNDrop::DropEvent& event) {
    AWindowBase::onDragDrop(event);

    for (const auto& [k, v] : event.data.data()) {
        ALogger::info("Drop") << "[" << k << "] = " << AString::fromUtf8(v);
    }

    auto surface = createOverlappingSurface({ 0, 0 }, { 100, 100 }, false);
    _<AViewContainer> popup = Vertical {
        Label { "Drop event" } with_style {
          FontSize { 18_pt },
          ATextAlign::CENTER,
        },
        [&]() -> _<AView> {
            if (auto u = event.data.urls()) {
                auto url = u->first();
                if (auto icon = ADesktop::iconOfFile(url.path())) {
                    return Centered { _new<ADrawableView>(icon) with_style { FixedSize { 64_dp } } };
                }
            }
            return nullptr;
        }(),
        AText::fromString("Caught drop event. See the logger output for contents.") with_style {
          ATextAlign::CENTER, MinSize { 100_dp, 40_dp } },
        Centered { Button { "OK" }.clicked(this, [surface] { surface->close(); }) }
    };
    ALayoutInflater::inflate(surface, popup);
    popup->pack();

    surface->setOverlappingSurfaceSize(popup->getSize());
    surface->setOverlappingSurfacePosition((getSize() - popup->getSize()) / 2);
}

bool ExampleWindow::onDragEnter(const ADragNDrop::EnterEvent& event) { return true; }
