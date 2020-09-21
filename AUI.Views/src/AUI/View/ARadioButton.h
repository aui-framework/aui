//
// Created by alex2 on 21.09.2020.
//

#pragma once

#include "ALabel.h"
#include "AViewContainer.h"

class API_AUI_VIEWS ARadioButton : public AViewContainer {
private:
    _<ALabel> mText;
    bool mChecked = false;

public:
    ARadioButton();
    ARadioButton(const AString& text);

    virtual ~ARadioButton();

    void setText(const AString& text);


    [[nodiscard]] bool isChecked() const {
        return mChecked;
    }

    void setChecked(const bool checked) {
        mChecked = checked;
        emit customCssPropertyChanged();
    }

    void getCustomCssAttributes(AMap<AString, AVariant>& map) override;
    void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

signals:
    emits<bool> checked;
};