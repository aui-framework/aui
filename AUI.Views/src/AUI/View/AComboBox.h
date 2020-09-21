//
// Created by alex2 on 21.09.2020.
//

#pragma once


#include "AButton.h"
#include <AUI/Model/IListModel.h>

class AComboBoxWindow;
class API_AUI_VIEWS AComboBox: public AButton {
private:
    _<IListModel<AString>> mModel;
    int mSelectionId = 0;
    bool mPopup = false;
    _<AComboBoxWindow> mComboWindow;
    bool mClickConsumer = false;

public:
    explicit AComboBox(const _<IListModel<AString>>& model);
    AComboBox();
    ~AComboBox() override;

    void setModel(const _<IListModel<AString>>& model);
    void render() override;
    void setSelectionId(int id);
    int getContentMinimumWidth() override;

    void getCustomCssAttributes(AMap<AString, AVariant>& map) override;

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    void destroyWindow();
};


