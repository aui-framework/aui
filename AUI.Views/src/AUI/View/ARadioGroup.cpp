/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 21.09.2020.
//

#include <AUI/Layout/AVerticalLayout.h>
#include "ARadioGroup.h"
#include "ARadioButton.h"
#include <AUI/Platform/AWindow.h>

ARadioGroup::ARadioGroup(const _<IListModel<AString>>& model):
    ARadioGroup()
{
    setModel(model);
}

ARadioGroup::ARadioGroup():
        mGroup(_new<ARadioButton::Group>())
{
    connect(mGroup->selectionChanged, this, [&](int id) {
        emit selectionChanged(AModelIndex(id));
    });
}


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

    if (auto w = getWindow())
        w->updateLayout();
    AWindow::current()->flagRedraw();
}