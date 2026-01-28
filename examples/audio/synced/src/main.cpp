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

#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Platform/AMessageBox.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ACheckBox.h"
#include "AUI/View/AGroupBox.h"

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AForEachUI.h>
#include "AUI/View/ARadioButton.h"
#include "AUI/View/ASlider.h"

#include <range/v3/view/cartesian_product.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/iota.hpp>

using namespace ass;
using namespace declarative;

static constexpr auto SAMPLES = std::array {
    "Dire Shred Cover Pack (credit: Youssry Askar)",
};

class GainAnalysis: public AObject, public ISoundInputStream {
public:
    explicit GainAnalysis(_<ISoundInputStream> inner) : mInner(std::move(inner)) {}

    size_t read(char* dst, size_t size) override {
        auto r = mInner->read(dst, size);
        if (r == 0) {
            return r;
        }

        const auto inputFormat = info();
        size_t sampleCount = size / aui::audio::bytesPerSample(inputFormat.sampleFormat);
        mSamplesBuffer.resize(sampleCount);
        aui::audio::convertSampleFormat(inputFormat.sampleFormat, ASampleFormat::F32,
                                      dst, reinterpret_cast<char*>(mSamplesBuffer.data()), sampleCount);

        int channelCount = int(inputFormat.channelCount);
        float prevFrame = std::numeric_limits<float>::infinity();
        float accumulator = 0.f;
        for (int channel = 0; channel < channelCount; ++channel) {
            for (size_t i = 0; i < mSamplesBuffer.size(); i += channelCount) {
                float currentFrame = mSamplesBuffer[i + channel];
                AUI_DEFER { prevFrame = currentFrame; };
                if (prevFrame == std::numeric_limits<float>::infinity()) {
                    continue;
                }
                accumulator += glm::distance(currentFrame, prevFrame);
            }
        }
        accumulator = glm::sqrt(glm::clamp(accumulator / mSamplesBuffer.size() * 100.f));
        lastFrame = accumulator;
        return r;
    }

    AAudioFormat info() override {
        return mInner->info();
    }

    ~GainAnalysis() override = default;

    AProperty<float> lastFrame;

private:
    _<ISoundInputStream> mInner;
    std::vector<float> mSamplesBuffer;
};

struct State {
    AProperty<int> sample = 0;

    struct Track {
        _<IAudioPlayer> player;
        _<GainAnalysis> gainAnalysis;

        AProperty<aui::audio::VolumeLevel> volume = 255;
    };

    AProperty<AVector<_<Track>>> tracks;
    AProperty<bool> isPlaying = false;
    AProperty<bool> isLoop = true;

    State() {
        AObject::connect(sample, AObject::GENERIC_OBSERVER, [this] {
            setup();
        });
        AObject::connect(isPlaying, AObject::GENERIC_OBSERVER, [this](bool isPlaying) {
            for (const auto& track : *tracks) {
                isPlaying ? track->player->play() : track->player->pause();
            }
        });
    }

    ~State() {
        stopAllPlayers();
    }

    void stopAllPlayers() {
        isPlaying = false;
        for (const auto& track : *tracks) {
            track->player->stop();
        }
        tracks.writeScope()->clear();
    }

    void setup() {
        stopAllPlayers();
        for (const auto& i : ranges::view::iota(0, 3)) {
            auto stream = _new<GainAnalysis>(ISoundInputStream::fromUrl(AUrl(":samples/{}/{}.ogg"_format(*sample, i))));
            auto track = aui::ptr::manage_shared(new Track {
                .player = IAudioPlayer::fromStream(stream),
                .gainAnalysis = std::move(stream),
            });
            AObject::connect(track->player->finished, AObject::GENERIC_OBSERVER, [this] {
                setup();
                if (isLoop) {
                    isPlaying = true;
                }
            });
            AObject::connect(track->volume, track->player, [&player = *track->player](aui::audio::VolumeLevel volume) {
                player.setVolume(volume);
            });
            tracks << std::move(track);
        }
    }
};

_<AView> radioButtons(_<State> state) {
    return AUI_DECLARATIVE_FOR(i, SAMPLES | ranges::view::enumerate, AVerticalLayout) {
        const auto& [index, text] = i;
        return RadioButton {
            .checked = AUI_REACT(state->sample == index),
            .onClick = [state, index] { state->sample = index; },
            .content = Label { text },
        };
    };
}

AUI_ENTRY {
    auto window = _new<AWindow>("Synced audio playback", 600_dp, 300_dp);
    auto state = _new<State>();
    window->setContents(
      Vertical {
          Horizontal {
              Button { Label { "Play" }, [state] { state->isPlaying = true; } } AUI_LET {
                  AObject::connect(AUI_REACT(!state->isPlaying), AUI_SLOT(it)::setEnabled);
              },
              Button { Label { "Pause" }, [state] { state->isPlaying = false; } } AUI_LET {
                  AObject::connect(AUI_REACT(state->isPlaying), AUI_SLOT(it)::setEnabled);
              },
              CheckBox {
                  .checked = AUI_REACT(state->isLoop),
                  .onCheckedChange = [=](bool v) { state->isLoop = v; },
                  .content = Label { "Loop" },
              },
          } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
          GroupBox {
              Label { "Sample" },
              radioButtons(state),
          },
          GroupBox {
              Label { "Tracks" },
              Vertical {
                  aui::detail::makeForEach([=]() -> decltype(auto) { return ranges::view::cartesian_product(*state->tracks | ranges::view::enumerate, ranges::view::iota(0, 2));  }, std::make_unique<AAdvancedGridLayout>(2, 1)) - [=](const auto& value) -> _<AView> {
                      const auto&[layerAndPlayer, column] = value;
                      const auto&[layer, track] = layerAndPlayer;
                      switch (column) {
                          case 0: return Horizontal {
                              _new<AView>() AUI_LET {
                                  it->setFixedSize({10_dp, 0});
                                  AObject::connect(track->gainAnalysis->lastFrame, it, [&it = *it](float frame) {
                                      it AUI_OVERRIDE_STYLE {
                                          BackgroundSolid { glm::mix(glm::vec4(AColor::BLACK), glm::vec4(AColor::GREEN), frame) },
                                      };
                                  });
                              },

                              Centered { Label { layer == 0 ? "Base" : "Layer {}"_format(layer) } },
                          } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } };
                          case 1: return Slider {
                              .value = AUI_REACT(aui::float_within_0_1(float(*track->volume) / 255.f)),
                              .onValueChanged = [=](float v) { track->volume = v * 255.f; },
                          };
                      }
                      return nullptr;
                  } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
              },
          },
      } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } }
    );
    window->show();
    return 0;
}

