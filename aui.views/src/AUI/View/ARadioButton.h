// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#pragma once

#include "ALabel.h"
#include "AViewContainer.h"
#include <AUI/Common/AMap.h>
#include <AUI/ASS/Selector/Selected.h>


class ARadioButtonInner: public AView
{
public:
    ARadioButtonInner() = default;
    virtual ~ARadioButtonInner() = default;

    void update();
};


/**
 * @brief A radio button.
 * @ingroup useful_views
 * @details
 * A radio button is a checkable button (similar to ACheckBox) that typically used in @ref ARadioGroup "groups". In a
 * group of radio buttons, only one radio button at a time can be checked thus checking another radio button in a group
 * causes to switch off the previous one.
 *
 * Whenever the radio button is checked or unchecked, it emits checked() signal.
 */
class API_AUI_VIEWS ARadioButton : public AViewContainer, public ass::ISelectable {
private:
    _<ALabel> mText;
    bool mChecked = false;
protected:
    bool selectableIsSelectedImpl() override;

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
        emit ARadioButton::checked(checked);
    }

    void onPointerReleased(const APointerReleasedEvent& event) override;

    class API_AUI_VIEWS Group: public AObject {
    private:
        AMap<int, _<ARadioButton>> mButtons;
        _weak<ARadioButton> mSelectedRadio;
        int mSelectedId = -1;

    public:
        Group() = default;
        ~Group() override = default;

        _<ARadioButton> addRadioButton(const _<ARadioButton>& radio, int id = -1);

        [[nodiscard]] _<ARadioButton> getSelectedRadio() const;
        [[nodiscard]] int getSelectedId() const;

        void setSelectedId(int id);

        [[nodiscard]] bool isSelected() const {
            return mSelectedRadio.lock() != nullptr;
        }

        void uncheckAll() {
            for (auto& b : mButtons) {
                b.second->setChecked(false);
            }
            mSelectedId = -1;
        }

    signals:
        emits<int> selectionChanged;
    };

signals:
    emits<bool> checked;
};

namespace declarative {
    using RadioButton = aui::ui_building::view<ARadioButton>;
}