//
// Created by alex2 on 21.09.2020.
//

#include <AUI/Layout/AVerticalLayout.h>
#include "ARadioGroup.h"
#include "ARadioButton.h"
#include <AUI/Platform/AWindow.h>

ARadioGroup::ARadioGroup(const _<IListModel<AString>>& model):
    mGroup(_new<ARadioButton::Group>())
{
    setModel(model);

    connect(mGroup->selectionChanged, this, [&](int id) {
        emit selectionChanged(AModelIndex(id));
    });

}

ARadioGroup::ARadioGroup():
        mGroup(_new<ARadioButton::Group>())
{

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
                auto r = _new<ARadioButton>(row);
                mGroup->addRadioButton(r);
                addView(r);
            }
            updateLayout();
        });
    }
    updateLayout();
    AWindow::current()->flagRedraw();
}