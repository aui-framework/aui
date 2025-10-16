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

//#ifdef _DEBUG

#include "DevtoolsPanel.h"
#include "AUI/Devtools/DevtoolsLayoutTab.h"
#include "AUI/Devtools/DevtoolsPerformanceTab.h"
#include "AUI/Devtools/DevtoolsPointerInspect.h"
#include "AUI/Platform/AWindowBase.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/ATabView.h"
#include "DevtoolsProfilingOptions.h"
#include "DevtoolsThreadsTab.h"

using namespace declarative;

DevtoolsPanel::DevtoolsPanel(AWindowBase* targetWindow) {
    auto tabs = _new<ATabView>();
    tabs->setExpanding();

    tabs->addTab(_new<DevtoolsLayoutTab>(targetWindow), "Layout");
    tabs->addTab(_new<DevtoolsPerformanceTab>(targetWindow), "Performance");
    tabs->addTab(_new<DevtoolsProfilingOptions>(targetWindow), "Other");
    tabs->addTab(_new<DevtoolsPointerInspect>(targetWindow), "Pointer inspect");
    tabs->addTab(_new<DevtoolsThreadsTab>(AThreadPool::global()), "Task queues");

    setContents(Centered { tabs });
}
