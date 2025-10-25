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
class TimerWindow : public AWindow {
public:
    TimerWindow() : AWindow("AUI - 7GUIs - Timer", 300_dp, 50_dp) {
        setContents(Centered {
          Vertical::Expanding {
            Horizontal {
              Label { "Elapsed Time:" },
              Centered::Expanding {
                _new<AProgressBar>() AUI_LET {
                        it & mElapsedTimeRatio;
                        it->setCustomStyle({ Expanding { 1, 0 } });
                    },
              },
            } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
            Label { AUI_REACT("{:.1f}s"_format(duration_cast<milliseconds>(*mElapsedTime).count() / 1000.f)) },
            Horizontal {
              Label { "Duration:" },
              _new<ASlider>() AUI_LET {
                      // it&& mDuration.biProjected(...) ;
                      it->setCustomStyle({ Expanding {} });
                  },
            } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
            _new<AButton>("Reset Timer") AUI_WITH_STYLE {
                  Expanding { 1, 0 },
                } AUI_LET { connect(it->clicked, me::reset); },
          } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
        });

        connect(mTimer->fired, me::update);
        mTimer->start();
    }

private:
    _<ATimer> mTimer = _new<ATimer>(100ms);
    high_resolution_clock::time_point mStartTime = high_resolution_clock::now();
    AProperty<high_resolution_clock::time_point> mCurrentTime;
    AProperty<high_resolution_clock::duration> mDuration = 30s;

    APropertyPrecomputed<high_resolution_clock::duration> mElapsedTime = [&] {
        return std::min(mCurrentTime - mStartTime, *mDuration);
    };

    APropertyPrecomputed<aui::float_within_0_1> mElapsedTimeRatio = [&] {
        return float(mElapsedTime->count()) / float(mDuration->count());
    };

    void update() { mCurrentTime = high_resolution_clock::now(); }

    void reset() { mStartTime = high_resolution_clock::now(); }
};
/// [example]

AUI_ENTRY {
    _new<TimerWindow>()->show();
    return 0;
}