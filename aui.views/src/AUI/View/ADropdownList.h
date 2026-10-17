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


#include "AButton.h"
#include <AUI/Model/IListModel.h>
#include <AUI/Platform/AOverlappingSurface.h>

/**
 * @brief A button with dropdown list.
 *
 * ![](imgs/views/ADropdownList.png)
 *
 * @ingroup views_input
 * @details
 * Shows the selected item as a button; a click opens a popup with all the items. The items come from a list model of
 * strings (`AListModel<AString>` is the simplest one).
 *
 * <!-- aui:snippet examples/ui/views/src/ExampleWindow.cpp dropdown -->
 *
 * `selectionId()` is a property with the index of the selected item (starting from 0, the first item is selected by
 * default). Bind it or listen to `selectionChanged`:
 *
 * ```cpp
 * auto list = _new<ADropdownList>(AListModel<AString>::make({ "one-way flight", "return flight" }));
 * AObject::connect(list->selectionId().changed, AObject::GENERIC_OBSERVER, [](int index) {
 *     ALogger::info("App") << "Selected " << index;
 * });
 * ```
 *
 * The model can be replaced later with `setModel`. Like any other view, the list can be disabled with `setDisabled()`.
 */
class API_AUI_VIEWS ADropdownList: public AButton {
private:
    _<IListModel<AString>> mModel;
    int mSelectionId = 0;
    bool mPopup = false;
    _weak<AOverlappingSurface> mComboWindow;

protected:
    virtual void updateText();
    virtual void onComboBoxWindowCreated();

    _<AViewContainer> comboWindow() {
        return mComboWindow.lock();
    }

public:
    explicit ADropdownList(const _<IListModel<AString>>& model);
    ADropdownList();
    ~ADropdownList() override;

    /**
     * @brief Selected id property.
     */
    auto selectionId() const {
        return APropertyDef {
            this,
            &ADropdownList::getSelectionId,
            &ADropdownList::setSelectionId,
            selectionChanged,
        };
    }

    void setModel(const _<IListModel<AString>>& model);
    void render(ARenderContext context) override;

    [[nodiscard]] int getSelectionId() const {
        return mSelectionId;
    }
    int getSelectedId() const {
        return mSelectionId;
    }
    void setSelectionId(int id);
    glm::ivec2 onIntrinsicMeasure(AConstraints constraints) override;
    AMinMaxAxis onComputeIntrinsicMinMaxAxis(int height) override;

    void onPointerReleased(const APointerReleasedEvent& event) override;

    void destroyWindow();

    [[nodiscard]]
    const _<IListModel<AString>>& getModel() const {
        return mModel;
    }

signals:
    emits<int> selectionChanged;
};

