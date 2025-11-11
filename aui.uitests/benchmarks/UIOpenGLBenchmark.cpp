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
#include "AUI/Platform/ARenderingContextOptions.h"
#include "AUI/Image/png/PngImageLoader.h"
#include "AUI/GL/OpenGLRenderer.h"

static void OpenGLRendering(benchmark::State& state) {

    ARenderingContextOptions::set({
      .initializationOrder {
        ARenderingContextOptions::OpenGL {},
      },
      .flags = ARenderContextFlags::NO_SMOOTH | ARenderContextFlags::NO_VSYNC,
    });

    _<AWindow> window;
    try {
        window = _new<AWindow>();
        window->show();
    } catch (const AException& e) {
        ALogger::info("OpenGLRendererTest") << "GPU is not available; skipping test\n" << e;
        return;
    }

    window->setContents(declarative::Centered { uiBenchmarkScene() });
    window->pack();

    AUI_ASSERT(dynamic_cast<OpenGLRenderer*>(&AWindow::current()->getRenderingContext()->renderer()));

    for (auto _ : state) {
        window->redraw();
    }
    PngImageLoader::save(AFileOutputStream("benchmark_OpenGLRendering.png"), window->getRenderingContext()->makeScreenshot());
}

BENCHMARK(OpenGLRendering);
