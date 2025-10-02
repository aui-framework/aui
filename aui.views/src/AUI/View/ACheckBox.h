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
 * ---
 * title: Check box
 * icon: material/checkbox-outline
 * ---
 *
 * @brief Checkboxes let users select one or more items from a list, or turn an item on or off.
 *
 * ![](imgs/views/ACheckBoxWrapper.png)
 *
 * @ingroup views_input
 * @details
 * A check box is an input control that allows the user to select or deselect an option. Unlike ARadioButton
 * buttons—which are mutually exclusive within a group, multiple check boxes may be checked independently.
 *
 * ![](imgs/views/ACheckBox.png)
 *
 * Use check boxes to enable or disable features, options, or actions in your UI.
 *
 * ## Create a basic check box
 *
 * The following code snippet shows a minimal check box implementation:
 *
 * <!-- aui:snippet examples/ui/checkbox1/src/main.cpp ACheckBox_example -->
 *
 * ### Key points about this code
 *
 * - `struct State` holds a reactive property `checked` representing the check box's state. When `checked` changes, the
 *    UI updates reactively.
 * - The `CheckBox` view binds `.checked` to the state's property, so the check mark is kept in sync with the data.
 * - `.onCheckedChange` toggles the state when the check box is clicked.
 * - Content (label or any view) can be provided using `.content`.
 * - The UI updates automatically based on the state because of AUI's reactive system.
 *
 * ### Result
 *
 * This example produces the following component when unchecked:
 *
 * ![](imgs/Screenshot_20250929_094428.png)
 *
 * And this is how the same checkbox appears when checked:
 *
 * ![](imgs/Screenshot_20250929_094529.png)
 *
 * ## Advanced example
 *
 * The following is a more complex example of how you can implement checkboxes in your app. In this snippet, there is a
 * parent checkbox and a series of child checkboxes. When the user activates the parent checkbox, the app checks all
 * child checkboxes.
 *
 * <!-- aui:snippet examples/ui/checkbox2/src/main.cpp ACheckBox_example -->
 *
 * ### Key points about this code
 *
 * - **State Management**:
 *     - A `State` structure keeps three Boolean properties (`option1`, `option2`, `option3`), each wrapped with
 *       `AProperty` for reactive (observable) updates.
 *     - The utility method `isAllOptionsSelected()` quickly checks if all options are selected.
 * - **"Select All" Parent Checkbox**:
 *     - The first `CheckBox` (labeled "Select all") is linked to `isAllOptionsSelected()` using a reactive binding
 *       (`AUI_REACT`).
 *     - When toggled, it sets all three state options to the new value, effectively selecting or deselecting all
 *       options at once.
 * - **Individual Option Checkboxes**:
 *     - Each subsequent `CheckBox` is bound to its individual option (`option1`, `option2`, `option3`) via `AUI_REACT`.
 *     - Changing any option updates its respective state property.
 * - **Synchronized Selection**:
 *     - If all individual checkboxes are checked, the "Select all" parent checkbox is also checked automatically (and
 *       vice versa).
 * - **Dynamic Status Label**:
 *     - A `Label` below the checkbox group reflects the selection status in real-time (shows "All options selected" or
 *       "Please select all options") using a reactive binding.
 *
 * ### Result
 *
 * This example produces the following component when all checkboxes are unchecked.
 *
 * ![](imgs/Screenshot_20250929_095124.png)
 *
 * Likewise, this is how the component appears when all options are checked, as when the user taps select all:
 *
 * ![](imgs/Screenshot_20250929_095204.png)
 *
 * When only one option is checked the parent checkbox appears unchecked:
 *
 * ![](imgs/Screenshot_20250929_095243.png)
 *
 * ## Styling
 *
 * Both `ACheckBox` and `ACheckBox::Box` are exposed only for styling purposes.
 *
 * <!-- aui:snippet aui.views/src/AUI/ASS/AStylesheet.cpp ACheckBox -->
 */
class API_AUI_VIEWS ACheckBox : public AViewContainerBase {
public:
    ACheckBox() : ACheckBox(nullptr) {}
    ACheckBox(_<AView> content);

    ~ACheckBox() override = default;

    [[nodiscard]] [[deprecated("use checked() instead.")]]
    bool isChecked() const {
        return mBox->checked;
    }

    [[deprecated("use checked() = true instead.")]]
    void setChecked(bool checked) {
        mBox->checked = checked;
    }

    [[nodiscard]]
    auto& checked() const {
        return mBox->checked;
    }

    [[nodiscard]]
    const auto& box() const {
        return mBox;
    }

    /**
     * @brief Represents the box/drawing part of the checkbox itself; without labels or other decorations.
     *
     * Exposed for styling purposes.
     */
    class Box : public AView, public ass::ISelectable {
    public:
        AProperty<bool> checked;
        emits<bool> userCheckedChange;

        Box();
        virtual ~Box() = default;

    protected:
        bool selectableIsSelectedImpl() override { return checked; }
    };

private:
    _<Box> mBox;
};

namespace declarative {
/**
 * <!-- aui:no_dedicated_page -->
 */
struct CheckBox {
    /**
     * @brief Whether or not the check box is checked.
     */
    contract::In<bool> checked;

    /**
     * @brief Handler for check box click event.
     * @param checked New state of the check box.
     * @details
     * Called when user checks/unchecks the check box.
     */
    contract::Slot<bool /* checked */> onCheckedChange;

    /**
     * @brief View associated with the check box.
     * @details
     * Can be any view, i.e., `Label` to display text.
     *
     * `content` view will be wrapped with ACheckBox.
     *
     * Clicking this view will toggle the check box.
     */
    _<AView> content;

    API_AUI_VIEWS _<AView> operator()();
};
}   // namespace declarative

/* legacy */
template <>
struct ADataBindingDefault<ACheckBox, bool> {
public:
    static auto property(const _<ACheckBox>& view) { return view->checked(); }
    static void setup(const _<ACheckBox>& view) {}

    static auto getGetter() { return &ACheckBox::checked; }

    static auto getSetter() { return &ACheckBox::setChecked; }
};

