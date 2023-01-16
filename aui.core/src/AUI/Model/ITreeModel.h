// AUI Framework - Declarative UI toolkit for modern C++17
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
// Created by alex2 on 7/1/2021.
//


#pragma once

#include <AUI/Common/ASignal.h>
#include "AModelRange.h"

class ATreeIndex {
private:
    void* mUserData;

public:
    ATreeIndex() = default;

    explicit ATreeIndex(void* userData):
            mUserData(userData)
    {
    }

    template<typename T>
    [[nodiscard]]
    T* getUserData() const {
        return reinterpret_cast<T*>(mUserData);
    }
};

template<typename T>
class ITreeModel
{
protected:
    virtual void* rootUserData() = 0;

public:
    virtual ~ITreeModel() = default;

    virtual size_t childrenCount(const ATreeIndex& parent) = 0;
    virtual T itemAt(const ATreeIndex& index) = 0;
    virtual ATreeIndex indexOfChild(size_t row, size_t column, const ATreeIndex& parent) = 0;

    ATreeIndex root() {
        return ATreeIndex{rootUserData()};
    }

    using stored_t = T;

signals:
};
