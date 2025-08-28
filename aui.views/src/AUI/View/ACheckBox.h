/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
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
 *
 * ![](imgs/views/ACheckBox.png)
 *
 * @ingroup views_input
 * @details
 * Checkbox is a checkable button that is typically used to enable/disable some action.
 *
 * For a checkbox with label, see [ACheckBoxWrapper].
 *
 * Whenever the checkbox is checked or unchecked, it emits checked() signal.
 */
class API_AUI_VIEWS ACheckBox : public AView, public ass::ISelectable {
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

    void setChecked(bool checked = true) {
        mChecked = checked;
        emit customCssPropertyChanged();
        emit ACheckBox::mCheckedChanged(checked);
    }

    void setUnchecked(bool unchecked = true) {
        setChecked(!unchecked);
    }

    bool consumesClick(const glm::ivec2& pos) override;

protected:
    bool selectableIsSelectedImpl() override;

private:
    bool mChecked = false;
    emits<bool> mCheckedChanged;
};


/**
 * @brief View container with a checkbox.
 *
 * ![](imgs/views/ACheckBoxWrapper.png)
 * @ingroup views_input
 * @details
 * For a checkbox itself, see [ACheckBox].
 */
class API_AUI_VIEWS ACheckBoxWrapper: public AViewContainerBase {
public:
    /**
     * @brief Construct ACheckBoxWrapper with a view.
     * @param viewToWrap view to wrap.
     */
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

    void setChecked(bool checked = true) {
        mCheckBox->setChecked(checked);
    }

    void setUnchecked(bool unchecked = true) {
        setChecked(!unchecked);
    }

private:
    _<ACheckBox> mCheckBox;

};


template<>
struct ADataBindingDefault<ACheckBox, bool> {
public:
    static auto property(const _<ACheckBox>& view) {
        return view->checked();
    }
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
    static auto property(const _<ACheckBoxWrapper>& view) {
        return view->checked();
    }
    static void setup(const _<ACheckBoxWrapper>& view) {}

    static auto getGetter() {
        return &ACheckBoxWrapper::checked;
    }

    static auto getSetter() {
        return &ACheckBoxWrapper::setChecked;
    }
};

namespace declarative {
    /**
     * @declarativeformof{ACheckBox}
     */
    struct CheckBox: aui::ui_building::view<ACheckBox> {
        CheckBox() = default;
    };

    /**
     * @declarativeformof{ACheckBoxWrapper}
     */
    struct CheckBoxWrapper: aui::ui_building::view<ACheckBoxWrapper> {
        /**
         * @brief Construct ACheckBoxWrapper with a view.
         * @param viewToWrap view to wrap.
         */
        explicit CheckBoxWrapper(const _<AView>& viewToWrap): aui::ui_building::view<ACheckBoxWrapper>(viewToWrap) {}
    };
}