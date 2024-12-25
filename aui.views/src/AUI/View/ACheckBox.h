/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

    [[nodiscard]]
    auto checked() const {
        return APropertyDef {
            .base = this,
            .get = &ACheckBox::mChecked,
            .set = &ACheckBox::setChecked,
            .changed = checkedChanged,
        };
    }

    void toggle() {
        setChecked(!checked());
    }

    void check() {
        setChecked(true);
    }

    void uncheck() {
        setChecked(false);
    }

    void setChecked(bool checked) {
        mChecked = checked;
        emit customCssPropertyChanged();
        emit ACheckBox::checkedChanged(checked);
    }

    void setUnchecked(bool unchecked) {
        setChecked(!unchecked);
    }

    bool consumesClick(const glm::ivec2& pos) override;


private:
    emits<bool> checkedChanged;
};


/**
 * @brief View container with a checkbox.
 * @ingroup userful_views
 */
class API_AUI_VIEWS ACheckBoxWrapper: public AViewContainerBase {
public:
    explicit ACheckBoxWrapper(const _<AView>& viewToWrap);

    [[nodiscard]]
    auto checked() const {
        return mCheckBox->checked();
    }

    void toggle() {
        setChecked(!checked());
    }

    void check() {
        setChecked(true);
    }

    void uncheck() {
        setChecked(false);
    }

    void setChecked(bool checked) {
        mCheckBox->setChecked(checked);
    }

    void setUnchecked(bool unchecked) {
        setChecked(!unchecked);
    }


private:
    _<ACheckBox> mCheckBox;
};

namespace declarative {
    using CheckBox = aui::ui_building::view<ACheckBox>;
    using CheckBoxWrapper = aui::ui_building::view<ACheckBoxWrapper>;
}