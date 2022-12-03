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
// Created by alex2 on 13.04.2021.
//


#pragma once


#include <AUI/Common/AObject.h>
#include <AUI/Common/AString.h>
#include <AUI/Model/IListModel.h>


class DemoListModel: public AObject, public IListModel<AString> {
private:
    size_t mListSize = 3;

public:
    virtual ~DemoListModel() = default;

    size_t listSize() override;
    AString listItemAt(const AModelIndex& index) override;

    void addItem();
    void removeItem();
};


