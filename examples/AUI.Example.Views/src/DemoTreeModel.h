// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

    size_t childrenCount(const ATreeIndex& parent) override;
    AString itemAt(const ATreeIndex& index) override;
    ATreeIndex indexOfChild(size_t row, size_t column, const ATreeIndex& parent) override;

protected:
    void *rootUserData() override;

};


