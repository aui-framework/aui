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
// Created by alex2 on 5/15/2021.
//


#pragma once


#include <AUI/Platform/AInput.h>
#include <AUI/Common/AVector.h>

class AShortcut {
private:
    AVector<AInput::Key> mKeys;

public:
    AShortcut() = default;
    AShortcut(const AVector<AInput::Key>& keys) : mKeys(keys) {}
    AShortcut(AVector<AInput::Key>&& keys) : mKeys(keys) {}

    const AVector<AInput::Key>& getKeys() const {
        return mKeys;
    }

    operator AString() const {
        AString s;
        for (auto& k : mKeys) {
            if (!s.empty()) {
                s += "+";
            }
            s += AInput::getName(k);
        }
        return s;
    }
    bool empty() const {
        return mKeys.empty();
    }
};

inline AShortcut operator+(const AInput::Key& k1, const AInput::Key& k2) {
    return AShortcut({k1, k2});
}

inline AShortcut operator+(const AShortcut& k1, const AInput::Key& k2) {
    auto k = k1.getKeys();
    k << k2;
    return AShortcut(std::move(k));
}

