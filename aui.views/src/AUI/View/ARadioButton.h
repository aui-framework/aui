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
#include <AUI/Util/Declarative/Containers.h>

/**
 * ---
 * title: Radio button
 * icon: material/radiobox-marked
 * ---
 *
 * @brief Radio buttons let people select one option from a set of options.
 *
 * ![](imgs/views/ARadioGroup.png)
 *
 * @ingroup views_input
 * @details
 * A radio button is a checkable button (similar to ACheckBox) that typically used in groups. In a
 * group of radio buttons, only one radio button at a time can be checked thus checking another radio button in a group
 * causes to switch off the previous one.
 *
 * <!-- aui:steal_documentation declarative::RadioButton -->
 *
 * ## Create a basic radio button
 *
 * The following code snippet renders a list of radio buttons:
 *
 * <!-- aui:snippet examples/ui/radiobutton1/src/main.cpp ARadioButton_example -->
 *
 * ### Key points about this code
 *
 * - `radioButtons` represents the labels of the radio buttons and their respective values. In our case, it's `int`s
 *    with magic values, but we strongly encourage you to use `enum`s instead.
 * - `struct State` holds a reactive property `selection` that tracks the currently selected radio button index. Changes
 *    to `selection` automatically trigger UI updates because of the reactive system.
 * - `RadioButton` creates a radio button view.
 *     - Each radio button's `.checked` state is bound to whether `state->selection == index` using the `AUI_REACT`
 *       macro.
 *     - Clicking a radio button updates the state via a lambda: . `state->selection = index;`
 *     - The selected option label updates automatically in response to changes.
 *
 * ### Result
 *
 * ![](imgs/Screenshot_20250929_075357.png)
 *
 * ## Styling
 *
 * Both `ARadioButton` and `ARadioButton::Circle` are exposed only for styling purposes.
 *
 * <!-- aui:snippet aui.views/src/AUI/ASS/AStylesheet.cpp ARadioButton -->
 */
class API_AUI_VIEWS ARadioButton
  : public AViewContainerBase {
public:
    ARadioButton(): ARadioButton(nullptr) {}
    ARadioButton(_<AView> content);

    virtual ~ARadioButton();

    [[nodiscard]]
    [[deprecated("use checked() instead.")]]
    bool isChecked() const { return mCircle->checked; }

    [[deprecated("use checked() = true instead.")]]
    void setChecked(bool checked) {
        mCircle->checked = checked;
    }

    [[nodiscard]]
    auto& checked() const {
        return mCircle->checked;
    }

    [[nodiscard]]
    const auto& circle() const {
        return mCircle;
    }

    [[deprecated("use declarative::RadioButton instead.")]]
    class API_AUI_VIEWS Group : public AObject {
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

        [[nodiscard]] bool isSelected() const { return mSelectedRadio.lock() != nullptr; }

        void uncheckAll() {
            for (auto& b : mButtons) {
                b.second->setChecked(false);
            }
            mSelectedId = -1;
        }

    signals:
        emits<int> selectionChanged;
    };

    /**
     * @brief Represents a circle of radiobutton itself; without labels, text or other things.
     */
    class Circle : public AView, public ass::ISelectable {
    public:
        AProperty<bool> checked;

        Circle();
        virtual ~Circle() = default;

    protected:
        bool selectableIsSelectedImpl() override {
            return checked;
        }
    };

private:
    _<Circle> mCircle;
};

namespace declarative {

/**
 * <!-- aui:no_dedicated_page -->
 */
struct RadioButton {
    /**
     * @brief Whether or not the radio button is checked.
     */
    contract::In<bool> checked;

    /**
     * @brief Handler for button click event.
     * @details
     * Called when user activates the button.
     */
    contract::Slot<> onClick;

    /**
     * @brief View associated with the radio button.
     * @details
     * Can be any view, i.e., `Label` to display text.
     *
     * `content` view will be wrapped with ARadioButton.
     *
     * Clicking or activating this view will cause the radio button to be checked.
     */
    _<AView> content;

    API_AUI_VIEWS _<AView> operator()();
};
}   // namespace declarative
