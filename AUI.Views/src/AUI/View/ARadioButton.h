//
// Created by alex2 on 21.09.2020.
//

#pragma once

#include "ALabel.h"
#include "AViewContainer.h"


class ARadioButtonInner: public AView
{
public:
    ARadioButtonInner() = default;
    virtual ~ARadioButtonInner() = default;

    void update();
};


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

    class Group: public AObject {
    private:
        AVector<_<ARadioButton>> mButtons;
        _weak<ARadioButton> mSelected;

    public:
        Group() = default;
        ~Group() override = default;

        void addRadioButton(_<ARadioButton> radio);

        [[nodiscard]] _<ARadioButton> getSelectedRadio() const;
        [[nodiscard]] int getSelectedId() const;

        [[nodiscard]] bool isSelected() const {
            return mSelected.lock() != nullptr;
        }
    signals:
        emits<int> selectionChanged;
    };

signals:
    emits<bool> checked;
};