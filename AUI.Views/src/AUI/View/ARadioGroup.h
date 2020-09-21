//
// Created by alex2 on 21.09.2020.
//

#pragma once

#include "AViewContainer.h"
#include "ARadioButton.h"
#include <AUI/Model/IListModel.h>

class API_AUI_VIEWS ARadioGroup: public AViewContainer {
private:
    _<IListModel<AString>> mModel;
    _<ARadioButton::Group> mGroup;

public:
    explicit ARadioGroup(const _<IListModel<AString>>& model);
    ARadioGroup();
    ~ARadioGroup() override;

    void setModel(const _<IListModel<AString>>& model);

    [[nodiscard]] bool isSelected() const {
        return mGroup->isSelected();
    }
    [[nodiscard]] int getSelectedId() const {
        return mGroup->getSelectedId();
    }

signals:
    emits<AModelIndex> selectionChanged;
};


