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

#include "AViewContainer.h"
#include "ARadioButton.h"
#include "AUI/Model/AListModel.h"
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Model/IListModel.h>

/**
 * @brief A group of radio buttons.
 *
 * ![](imgs/views/ARadioGroup.png)
 *
 * @ingroup views_input
 * @details
 * ARadioGroup initializes radio buttons by itself. Since ARadioGroup is a [view container](aviewcontainer.md) it
 * places the initialized radio buttons to itself.
 */
class API_AUI_VIEWS ARadioGroup : public AViewContainer {
private:
    _<IListModel<AString>> mModel;
    _<ARadioButton::Group> mGroup;

public:
    template <typename... RadioButtons>
    explicit ARadioGroup(RadioButtons&&... radioButtons) : ARadioGroup() {
        setLayout(std::make_unique<AVerticalLayout>());
        setViews({ std::forward<RadioButtons>(radioButtons)... });
    }
    ARadioGroup();
    ~ARadioGroup() override;

    /**
     * @brief Selected id property.
     */
    auto selectionId() const {
        return APropertyDef {
            this,
            &ARadioGroup::getSelectedId,
            &ARadioGroup::setSelectedId,
            selectionChanged,
        };
    }

    void setViews(AVector<_<AView>> views) {
        for (const _<AView>& v : views) {
            if (auto rb = _cast<ARadioButton>(v)) {
                mGroup->addRadioButton(rb);
                addView(v);
            }
        }
    }

    void setModel(const _<IListModel<AString>>& model);

    [[nodiscard]] bool isSelected() const { return mGroup->isSelected(); }
    [[nodiscard]] int getSelectedId() const { return mGroup->getSelectedId(); }
    void setSelectedId(int id) const;

signals:
    emits<int> selectionChanged;
};

namespace declarative {

/**
 * @declarativeformof{ARadioGroup}
 */
struct RadioGroup : aui::ui_building::view_container_layout<AVerticalLayout, ARadioGroup> {
    using aui::ui_building::view_container_layout<AVerticalLayout, ARadioGroup>::layouted_container_factory;
    struct Horizontal : aui::ui_building::view_container_layout<AHorizontalLayout, ARadioGroup> {
        using aui::ui_building::view_container_layout<AHorizontalLayout, ARadioGroup>::layouted_container_factory;
    };
};
}   // namespace declarative