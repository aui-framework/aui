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
    virtual bool isShouldBeCached(const K& key, const _<T>& image) {
		return true;
	}

public:
	
	static _<T> get(const K& key)
	{
		if (auto i = Container::inst().mContainer.contains(key))
		{
			return i->second;
		}
		Cache& i = Container::inst();
		auto value = i.load(key);
		if (i.isShouldBeCached(key, value)) {
			put(key, value);
		}
		return value;
	}
	
	static void put(const K& key, _<T> value)
	{
        Container::inst().mContainer[key] = value;
	}

    static void cleanup() {
        Container::inst().mContainer.clear();
    }
};
