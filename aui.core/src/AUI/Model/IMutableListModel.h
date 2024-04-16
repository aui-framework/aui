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

#pragma once

#include <AUI/Common/ASignal.h>
#include <AUI/Model/AListModelSelection.h>
#include "AListModelRange.h"

template<typename T>
class IRemovableListModel: public virtual IListModel<T>
{
public:
    virtual ~IRemovableListModel() = default;
    virtual void removeItems(const AListModelRange<T>& items) = 0;
    virtual void removeItems(const AListModelSelection<T>& items) {
        for (const auto& r : items.ranges()) {
            removeItems(r);
        }
    }
    virtual void removeItem(const AListModelIndex& item) = 0;

};


template<typename T>
class IValueMutableListModel: public virtual IListModel<T>
{
public:
    virtual ~IValueMutableListModel() = default;
    virtual void setItem(const AListModelIndex& index, const T& value) = 0;

};
