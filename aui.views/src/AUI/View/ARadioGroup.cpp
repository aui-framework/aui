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

//
// Created by alex2 on 21.09.2020.
//

#include <AUI/Layout/AVerticalLayout.h>
#include "ARadioGroup.h"
#include "ARadioButton.h"
#include <AUI/Platform/AWindow.h>


ARadioGroup::~ARadioGroup() {

}

void ARadioGroup::setModel(const _<IListModel<AString>>& model) {
    mModel = model;
    setLayout(_new<AVerticalLayout>());

    if (mModel) {
        for (size_t i = 0; i < model->listSize(); ++i) {
            auto r = _new<ARadioButton>(model->listItemAt(i));
            mGroup->addRadioButton(r);
            addView(r);
        }

        connect(mModel->dataInserted, this, [&](const AModelRange<AString>& data) {
            for (const auto& row : data) {
                auto r = _new<ARadioButton>(row.get());
                mGroup->addRadioButton(r);
                addView(r);
            }
            updateLayout();
        });
    }

    requestLayoutUpdate();
}