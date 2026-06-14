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

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Util/ARandom.h>
#include "DevtoolsThreadsTab.h"
#include <AUI/View/AScrollArea.h>

using namespace declarative;
using namespace ass;
using namespace std::chrono_literals;

namespace {

class WatcherView : public ALabel {
public:
    explicit WatcherView(std::function<size_t()> observe) : mObserve(std::move(observe)) {
        setCustomStyle({
          TextColor { AColor::BLACK },
          Padding { 4_dp, 8_dp },
          Margin { 0 },
        });
    }
    ~WatcherView() override = default;

    void render(ARenderContext ctx) override {
        drawColor(ctx);
        ALabel::render(ctx);
    }

    void update() {
        mCurrentValue = mObserve();
        setText("{}"_format(mCurrentValue));
    }

private:
    std::function<size_t()> mObserve;
    size_t mCurrentValue = 0;

    void drawColor(const ARenderContext& ctx) {
        if (mCurrentValue <= 0) {
            return;
        }
        RenderHints::PushColor s(ctx.render);

        auto color = AColor::WHITE;
        if (mCurrentValue < 5) {
            color = 0x73c69730_argb;
            goto paint;
        }
        if (mCurrentValue < 50) {
            color = 0xb8c69730_argb;
            goto paint;
        }

        if (mCurrentValue < 500) {
            color = 0xb8b03b07_argb;
            goto paint;
        }

        if (mCurrentValue < 5000) {
            color = 0xffb03b07_argb;
            goto paint;
        }

        color = 0xff0000_rgb;

    paint:
        ctx.render.rectangle(ASolidBrush { color }, { 0, 0 }, getSize());
    }
};
}   // namespace

DevtoolsThreadsTab::DevtoolsThreadsTab(AThreadPool& targetThreadPool) {
    mUpdateTimer = _new<ATimer>(100ms);

    auto views = _form({
      {
        Label { "Thread" } AUI_OVERRIDE_STYLE {
          FontSize { 10_pt },
          Expanding {},
        },
        Label { "Task Count" } AUI_OVERRIDE_STYLE {
          FontSize { 10_pt },
          FixedSize { 100_dp, {} },
        },
      },
    }) AUI_OVERRIDE_STYLE {
        LayoutSpacing { 1_px },
    };

    auto addThread = [&](const _<AAbstractThread>& thread) {
        views->addViews({
          _new<ALabel>(thread->threadName()),
          _new<WatcherView>([thread] {
              return thread->messageQueue().messages().size();
          }) AUI_LET { connect(mUpdateTimer->fired, AUI_SLOT(it)::update); },
        });
    };

    addThread(AThread::current());
    views->addViews({
      _new<ALabel>("Thread Pool"),
      _new<WatcherView>([&targetThreadPool] {
        return targetThreadPool.getTotalTaskCount();
      }) AUI_LET { connect(mUpdateTimer->fired, AUI_SLOT(it)::update); },
    });

    setContents(Stacked { AScrollArea::Builder().withContents(Vertical {
      std::move(views),
    }) });

    mUpdateTimer->start();
}
