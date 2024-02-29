// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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


