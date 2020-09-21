//
// Created by alex2 on 21.09.2020.
//

#pragma once


#include "AButton.h"
#include <AUI/Model/IListModel.h>

class API_AUI_VIEWS AComboBox: public AButton {
private:
    _<IListModel<AString>> mModel;
    int mSelectionId = 0;

public:
    explicit AComboBox(const _<IListModel<AString>>& model);
    AComboBox();
    ~AComboBox() override = default;

    void setModel(const _<IListModel<AString>>& model);

    void render() override;

    void setSelectionId(int id);

    int getContentMinimumWidth() override;
};


