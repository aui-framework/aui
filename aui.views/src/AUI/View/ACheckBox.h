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

#pragma once

#include "ALabel.h"
#include "AViewContainer.h"
#include <AUI/ASS/Selector/Selected.h>


/**
 * @brief A check box (without label).
 * @ingroup useful_views
 * @details
 * Checkbox is a checkable button that is typically used to enable/disable some action.
 *
 * Whenever the radio button is checked or unchecked, it emits checked() signal.
 */
class API_AUI_VIEWS ACheckBox : public AView, public ass::ISelectable {
private:
    bool mChecked = false;
protected:
    bool selectableIsSelectedImpl() override;

public:
    ACheckBox();

    void toggle() {
        setChecked(!isChecked());
    }

    [[nodiscard]] bool isChecked() const {
        return mChecked;
    }

    void check() {
        setChecked(true);
    }

    void uncheck() {
        setChecked(false);
    }

    void setChecked(bool checked = true) {
        mChecked = checked;
        emit customCssPropertyChanged();
        emit ACheckBox::checked(checked);
    }

    void setUnchecked(bool unchecked = true) {
        setChecked(!unchecked);
    }

    bool consumesClick() override;


signals:
    emits<bool> checked;
};


/**
 * @brief View container with a checkbox.
 * @ingroup userful_views
 */
class API_AUI_VIEWS ACheckBoxWrapper: public AViewContainer {
public:
    explicit ACheckBoxWrapper(const _<AView>& viewToWrap);

    void toggle() {
        setChecked(!isChecked());
    }

    [[nodiscard]] bool isChecked() const {
        return mCheckBox->isChecked();
    }

    void check() {
        setChecked(true);
    }

    void uncheck() {
        setChecked(false);
    }

    void setChecked(bool checked = true) {
        mCheckBox->setChecked(checked);
    }

    void setUnchecked(bool unchecked = true) {
        setChecked(!unchecked);
    }

private:
    _<ACheckBox> mCheckBox;

signals:
    emits<bool> checked;
};


template<>
struct ADataBindingDefault<ACheckBox, bool> {
public:
    static void setup(const _<ACheckBox>& view) {}

    static auto getGetter() {
        return &ACheckBox::checked;
    }

    static auto getSetter() {
        return &ACheckBox::setChecked;
    }
};


template<>
struct ADataBindingDefault<ACheckBoxWrapper, bool> {
public:
    static void setup(const _<ACheckBoxWrapper>& view) {}

    static auto getGetter() {
        return &ACheckBoxWrapper::checked;
    }

    static auto getSetter() {
        return &ACheckBoxWrapper::setChecked;
    }
};

namespace declarative {
    using CheckBox = aui::ui_building::view<ACheckBox>;
    using CheckBoxWrapper = aui::ui_building::view<ACheckBoxWrapper>;
}