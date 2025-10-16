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
// Created by alex2 on 21.09.2020.
//

#include "ARadioGroup.h"
#include "ARadioButton.h"
#include <AUI/Platform/AWindow.h>


ARadioGroup::ARadioGroup() : mGroup(_new<ARadioButton::Group>()) {
    connect(mGroup->selectionChanged, this, [&](int index) {
        emit selectionChanged(index);
    });
}

ARadioGroup::~ARadioGroup() {

}

void ARadioGroup::setModel(const _<IListModel<AString>>& model) {
    setLayout(std::make_unique<AVerticalLayout>());
    mModel = model;

    if (mModel) {
        for (size_t i = 0; i < model->listSize(); ++i) {
            auto r = _new<ARadioButton>(declarative::Label { model->listItemAt(i) });
            mGroup->addRadioButton(r);
            addView(r);
        }

        connect(mModel->dataInserted, this, [&](const AListModelRange<AString>& data) {
            for (const auto& row : data) {
                auto r = _new<ARadioButton>(declarative::Label { row.get() });
                mGroup->addRadioButton(r);
                addView(r);
            }
            applyGeometryToChildrenIfNecessary();
        });
    }

    markMinContentSizeInvalid();
}

void ARadioGroup::setSelectedId(int id) const {
    mGroup->setSelectedId(id);
}
