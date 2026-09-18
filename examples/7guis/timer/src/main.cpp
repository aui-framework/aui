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

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AUI/View/AProgressBar.h"
#include "AUI/View/ASlider.h"
#include "AUI/View/AButton.h"

using namespace declarative;

using namespace std::chrono;
using namespace std::chrono_literals;

static constexpr high_resolution_clock::duration MAX_DURATION = 60s;

/// [example]
struct TimerState {
    high_resolution_clock::time_point startTime = high_resolution_clock::now();
    AProperty<high_resolution_clock::time_point> currentTime;
    AProperty<high_resolution_clock::duration> duration = 30s;

    APropertyPrecomputed<high_resolution_clock::duration> elapsedTime = [this] {
        return std::min(currentTime - startTime, *duration);
    };

    APropertyPrecomputed<aui::float_within_0_1> elapsedTimeRatio = [this] {
        return float(elapsedTime->count()) / float(duration->count());
    };
};

class TimerWindow : public AWindow {
public:
    TimerWindow() : AWindow("AUI - 7GUIs - Timer", 300_dp, 50_dp) {
        auto state = _new<TimerState>();
        setContents(Centered {
          Vertical::Expanding {
            Horizontal {
              Label { "Elapsed Time:" },
              Centered::Expanding {
                ProgressBar {
                    .progress = AUI_REACT(*state->elapsedTimeRatio),
                } AUI_OVERRIDE_STYLE { Expanding { 1, 0 } },
              },
            } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
            Label { AUI_REACT("{:.1f}s"_format(duration_cast<milliseconds>(*state->elapsedTime).count() / 1000.f)) },
            Horizontal {
              Label { "Duration:" },
              Slider {
                .value = AUI_REACT(float(state->duration->count()) / float(MAX_DURATION.count())),
                .onValueChanged =
                    [state](aui::float_within_0_1 newValue) {
                        state->duration =
                            high_resolution_clock::duration(long(float(MAX_DURATION.count()) * float(newValue)));
                    },
              } AUI_OVERRIDE_STYLE { Expanding { 1, 0 } },
            } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
            _new<AButton>("Reset Timer") AUI_OVERRIDE_STYLE {
                  Expanding { 1, 0 },
                } AUI_LET { connect(it->clicked, [state] { state->startTime = high_resolution_clock::now(); }); },
          } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } },
        });

        mTimer = _new<ATimer>(100ms);
        connect(mTimer->fired, [state] { state->currentTime = high_resolution_clock::now(); });
        mTimer->start();
    }

private:
    _<ATimer> mTimer;
};
/// [example]

AUI_ENTRY {
    _new<TimerWindow>()->show();
    return 0;
}