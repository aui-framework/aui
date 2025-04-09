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
// Created by alex2 on 13.04.2021.
//


#pragma once


#include <AUI/Common/AObject.h>
#include <AUI/Common/AString.h>
#include <AUI/Model/IListModel.h>


class DemoListModel: public IListModel<AString> {
private:
    size_t mListSize = 3;

public:
    ~DemoListModel() override = default;

    size_t listSize() override;
    AString listItemAt(const AListModelIndex& index) override;

    void addItem();
    void removeItem();
};


