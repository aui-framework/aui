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
    friend class API_AUI_VIEWS ACheckBoxWrapper;
    friend struct ADataBindingDefault<ACheckBox, bool>;
public:
    ACheckBox();

    [[nodiscard]]
    auto checked() const {
        return APropertyDef {
            this,
            &ACheckBox::mChecked,
            &ACheckBox::setChecked,
            mCheckedChanged,
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

    bool consumesClick(const glm::ivec2& pos) override;

protected:
    bool selectableIsSelectedImpl() override;

private:
    bool mChecked = false;
    emits<bool> mCheckedChanged;

    void setChecked(bool checked) {
        mChecked = checked;
        emit customCssPropertyChanged();
        emit ACheckBox::mCheckedChanged(checked);
    }
};


/**
 * @brief View container with a checkbox.
 * @ingroup userful_views
 */
class API_AUI_VIEWS ACheckBoxWrapper: public AViewContainerBase {
    friend struct ADataBindingDefault<ACheckBoxWrapper, bool>;
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

    void setUnchecked(bool unchecked) {
        setChecked(!unchecked);
    }


private:
    _<ACheckBox> mCheckBox;

    void setChecked(bool checked) {
        mCheckBox->setChecked(checked);
    }
};


template<>
struct ADataBindingDefault<ACheckBox, bool> {
public:
    static void setup(const _<ACheckBox>& view) {}

    static auto getGetter() {
        return &ACheckBox::mCheckedChanged;
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