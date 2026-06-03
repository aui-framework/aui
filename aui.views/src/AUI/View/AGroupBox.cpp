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

//
// Created by Alex2772 on 7/14/2022.
//

#include "AGroupBox.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "ACheckBox.h"
#include <AUI/Render/ACanvas.hpp>

using namespace declarative;

AGroupBox::AGroupBox(_<AView> titleView, _<AView> contentView):
    mTitle(std::move(titleView)),
    mContent(std::move(contentView)) {


    setLayout(std::make_unique<AVerticalLayout>());

    using namespace declarative;
    setContents(Vertical {
        Horizontal { mTitle } << ".agroupbox-title",
        mFrame = Vertical {
            Vertical::Expanding {
                mContent AUI_LET { it->setExpanding(); }
            } << ".agroupbox-inner"
        } AUI_OVERRIDE_STYLE {
            Expanding {},
            AOverflow::HIDDEN,
        }
    });

    if (auto asCheckbox = _cast<ACheckBox>(mTitle)) {
        connect(asCheckbox->checked(), me::updateCheckboxState);
    } else if (auto asCheckbox = _cast<ACheckBox::Box>(mTitle)) {
        connect(asCheckbox->checked, me::updateCheckboxState);
    }
}

void AGroupBox::applyGeometryToChildren() {
    AViewContainerBase::applyGeometryToChildren();
}

void AGroupBox::updateCheckboxState(bool checked) {
    mFrame->setEnabled(checked);
}
