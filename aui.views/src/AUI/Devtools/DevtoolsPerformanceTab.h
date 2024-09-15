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

#pragma once

#include "AUI/Util/ADataBinding.h"
#include "AUI/View/ATreeView.h"
#include "AUI/Platform/ABaseWindow.h"
#include "ViewPropertiesView.h"
#include <variant>

class DevtoolsPerformanceTab: public AViewContainer {
public:
    DevtoolsPerformanceTab(ABaseWindow* targetWindow);

#if AUI_PROFILING
    struct Model {
        struct Running {};
        struct Paused {};

        using State = std::variant<Running, Paused>;
        State state = Running{};
    };
    ADataBinding<Model> mModel;

    void toggleRunPause() {
        if (std::holds_alternative<Model::Running>(mModel->state)) {
            mModel.setValue(&Model::state, Model::Paused{});
        } else {
            mModel.setValue(&Model::state, Model::Running{});
        }

    }
#endif


private:
    ABaseWindow* mTargetWindow;
#if AUI_PROFILING
    emits<APerformanceSection::Datas> nextFrame;
#endif

};
