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