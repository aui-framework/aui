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
 *
 * ![](imgs/views/ARadioButton.png)
 *
 * @ingroup views_input
 * @details
 * A radio button is a checkable button (similar to ACheckBox) that typically used in [groups](aradiogroup.md). In a
 * group of radio buttons, only one radio button at a time can be checked thus checking another radio button in a group
 * causes to switch off the previous one.
 *
 * Whenever the radio button is checked or unchecked, it emits checked() signal.
 */
class API_AUI_VIEWS ARadioButton : public AViewContainerBase, public ass::ISelectable {
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
        emit ARadioButton::mCheckedChanged(checked);
    }

    [[nodiscard]]
    auto checked() const {
        return APropertyDef {
            this,
            &ARadioButton::mChecked,
            &ARadioButton::setChecked,
            mCheckedChanged,
        };
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

protected:
    bool selectableIsSelectedImpl() override;

private:
    _<ALabel> mText;
    bool mChecked = false;
    emits<bool> mCheckedChanged;
};

namespace declarative {
    /**
     * @declarativeformof{ARadioButton}
     */
    struct RadioButton: aui::ui_building::view<ARadioButton> {
        using aui::ui_building::view<ARadioButton>::view;
    };
}