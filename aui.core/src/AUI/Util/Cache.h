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

#pragma once

#include "AUI/Common/AString.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/SharedPtr.h"

template<typename T, typename Container, typename K = AString>
class Cache
{
private:
	AMap<K, _<T>> mContainer;

protected:
    virtual _<T> load(const K& key) = 0;

public:
	
	static _<T> get(const K& key)
	{
		if (auto i = Container::inst().mContainer.contains(key))
		{
			return i->second;
		}
		Cache& i = Container::inst();
		auto value = i.load(key);
		put(key, value);
		return value;
	}
	
	static void put(const K& key, _<T> value)
	{
        Container::inst().mContainer[key] = value;
	}

    static void cleanup() {
        Container::inst(). mContainer.clear();
    }
};
