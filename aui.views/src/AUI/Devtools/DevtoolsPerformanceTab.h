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

#pragma once

#include "AUI/Util/ADataBinding.h"
#include "AUI/View/ATreeView.h"
#include "AUI/Platform/ASurface.h"
#include "ViewPropertiesView.h"
#include <variant>

class DevtoolsPerformanceTab: public AViewContainerBase {
public:
    DevtoolsPerformanceTab(ASurface* targetWindow);

#if AUI_PROFILING
    struct Running {};
    struct Paused {};

    using State = std::variant<Running, Paused>;
    AProperty<State> mState = Running{};

    void toggleRunPause() {
        if (std::holds_alternative<Running>(*mState)) {
            mState = Paused{};
        } else {
            mState = Running{};
        }
    }
#endif


private:
    ASurface* mTargetWindow;
#if AUI_PROFILING
    emits<APerformanceSection::Datas> nextFrame;
#endif

};
