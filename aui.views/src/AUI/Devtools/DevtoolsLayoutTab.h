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

#include "AUI/View/ATreeView.h"
#include "AUI/Platform/AWindowBase.h"
#include "ViewPropertiesView.h"

class DevtoolsLayoutTab: public AViewContainerBase {
public:
    DevtoolsLayoutTab(AWindowBase* targetWindow);
    ~DevtoolsLayoutTab() override;

private:
    AWindowBase* mTargetWindow;

    _<ATreeView> mViewHierarchyTree;
    _<ViewPropertiesView> mViewPropertiesView;

    void forceLayoutUpdate();
};
