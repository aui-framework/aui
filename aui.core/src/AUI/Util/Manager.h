/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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



