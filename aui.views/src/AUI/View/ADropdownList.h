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

//
// Created by alex2 on 21.09.2020.
//

#pragma once


#include "AButton.h"
#include <AUI/Model/IListModel.h>
#include <AUI/Platform/AOverlappingSurface.h>

/**
 * @brief A button with dropdown list.
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

    void setModel(const _<IListModel<AString>>& model);
    void render() override;

    [[nodiscard]] int getSelectionId() const {
        return mSelectionId;
    }
    int getSelectedId() const {
        return mSelectionId;
    }
    void setSelectionId(int id);
    int getContentMinimumWidth(ALayoutDirection layout) override;

    void onMouseReleased(const APointerReleasedEvent& event) override;

    void destroyWindow();

    [[nodiscard]]
    const _<IListModel<AString>>& getModel() const {
        return mModel;
    }

signals:
    emits<int> selectionChanged;
};


