/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2 on 7/1/2021.
//


#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Model/ITreeModel.h>

class DemoTreeModel: public ITreeModel<AString> {
public:
    ~DemoTreeModel() override = default;

    ATreeModelIndex root() override;

    size_t childrenCount(const ATreeModelIndex& vertex) override;

    AString itemAt(const ATreeModelIndex& index) override;

    ATreeModelIndex indexOfChild(size_t row, size_t column, const ATreeModelIndex& vertex) override;

    ATreeModelIndex parent(const ATreeModelIndex& vertex) override;

};


