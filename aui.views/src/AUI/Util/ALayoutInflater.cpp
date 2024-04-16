// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

//
// Created by Alex2772 on 11/12/2021.
//

#include <AUI/Layout/AStackedLayout.h>
#include "ALayoutInflater.h"
#include <AUI/Platform/ABaseWindow.h>

void ALayoutInflater::inflate(aui::no_escape<AViewContainer> root, const _<AView>& view) {
    root->setLayout(_new<AStackedLayout>());
    if (view) {
        view->setExpanding();
        root->addView(view);
    }
    AUI_NULLSAFE(root->getWindow())->flagUpdateLayout();
}
