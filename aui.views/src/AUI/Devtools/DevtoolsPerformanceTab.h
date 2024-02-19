// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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