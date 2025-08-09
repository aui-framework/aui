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
#include "ACheckBox.h"
#include "AUI/Layout/AHorizontalLayout.h"

using namespace declarative;

ACheckBox::ACheckBox()
{
    connect(clicked, me::toggle);
}


bool ACheckBox::consumesClick(const glm::ivec2& pos) {
    return true;
}

bool ACheckBox::selectableIsSelectedImpl() {
    return mChecked;
}

ACheckBoxWrapper::ACheckBoxWrapper(const _<AView>& viewToWrap) {
    setLayout(std::make_unique<AHorizontalLayout>());
    addView(Centered { mCheckBox = _new<ACheckBox>() });
    addView(viewToWrap);

    connect(clicked, me::toggle);

    mCheckBox->clicked.clearAllOutgoingConnectionsWith(mCheckBox); // fixes double toggle
}
