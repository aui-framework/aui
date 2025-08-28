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
 * ![](imgs/Views/ADropdownList.png)
 *
 * @ingroup useful_views
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
    int getContentMinimumWidth() override;

    void onPointerReleased(const APointerReleasedEvent& event) override;

    void destroyWindow();

    [[nodiscard]]
    const _<IListModel<AString>>& getModel() const {
        return mModel;
    }

signals:
    emits<int> selectionChanged;
};


