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
#include "AUI/Traits/any_range_view.h"

#include <AUI/Model/IListModel.h>
#include <AUI/Platform/AOverlappingSurface.h>
#include <AUI/Util/Declarative/Containers.h>


namespace declarative {
/**
 * @brief A button with dropdown list.
 *
 * ![](imgs/views/ADropdownList.png)
 *
 * @ingroup views_input
 */
struct API_AUI_VIEWS DropdownList {
    contract::In<AVector<AString>> items;
    contract::In<std::size_t> selectionId;
    contract::Slot<std::size_t> onSelectionChange;
    std::function<_<AView>(_<AView>)> body = defaultBody;
    std::function<_<AView>(contract::In<AString> text)> label = defaultLabel;
    _<AView> icon = defaultIcon();

    static _<AView> defaultBody(_<AView> content);
    static _<AView> defaultLabel(contract::In<AString> text);
    static _<AView> defaultIcon();

    _<AView> operator()();
};
}


