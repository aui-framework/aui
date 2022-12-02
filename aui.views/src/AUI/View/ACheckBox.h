/*
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
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

    bool consumesClick(const glm::ivec2& pos) override;


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