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

#include "AViewContainer.h"
#include "ARadioButton.h"
#include "AUI/Model/AListModel.h"
#include "AUI/Layout/AVerticalLayout.h"
#include <AUI/Model/IListModel.h>

/**
 * @brief A group of radio buttons.
 * @ingroup useful_views
 * @details
 * ARadioGroup initializes radio buttons by itself. Since ARadioGroup is a @ref AViewContainer "view container" it
 * places the initialized radio buttons to itself.
 */
class API_AUI_VIEWS ARadioGroup: public AViewContainer {
private:
    _<IListModel<AString>> mModel;
    _<ARadioButton::Group> mGroup;

public:
    template<typename... RadioButtons>
    explicit ARadioGroup(RadioButtons&&... radioButtons): mGroup(_new<ARadioButton::Group>()) {
        setLayout(_new<AVerticalLayout>());
        aui::parameter_pack::for_each([&](const _<ARadioButton>& v) {
            mGroup->addRadioButton(v);
            addView(v);
        }, std::forward<RadioButtons>(radioButtons)...);
    }
    ARadioGroup();
    ~ARadioGroup() override;

    void setModel(const _<IListModel<AString>>& model);

    [[nodiscard]] bool isSelected() const {
        return mGroup->isSelected();
    }
    [[nodiscard]] int getSelectedId() const {
        return mGroup->getSelectedId();
    }

signals:
    emits<AModelIndex> selectionChanged;
};


namespace declarative {
    struct RadioGroup: aui::ui_building::view<ARadioGroup> {
    public:
        using view<ARadioGroup>::view;

    };
}