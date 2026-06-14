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

#include <benchmark/benchmark.h>
#include "UIBenchmarkScene.h"

static void Layout(benchmark::State& state) {
    _<AWindow> window = _new<AWindow>();

    for (auto _2 : state) {
        _<AViewContainer> v = declarative::Centered { uiBenchmarkScene() };
        v->pack();
        v->applyGeometryToChildrenIfNecessary();
    }
}

BENCHMARK(Layout);
