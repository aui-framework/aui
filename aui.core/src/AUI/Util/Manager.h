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

#pragma once

#include "AUI/Common/AMap.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/Common/AString.h"


template<class T>
class Manager {
protected:
	AMap<AString, _<T>> mItems;
	virtual _ <T> newItem(const AString& name) = 0;
public:
	virtual ~Manager() {}
	_<T> get(AString name) {
		auto it = mItems.find(name);
		if (it == mItems.end()) {
			_ <T> p = this->newItem(name);
			mItems[name] = p;
			return p;
		}
		return it->second;
	}

	const std::map<AString, _<T>>& getItems() const {
		return mItems;
	}
};



