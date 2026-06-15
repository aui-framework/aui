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

#include <AUI/Common/SharedPtr.h>

template <typename BaseType>
class Factory
{
public:
	virtual ~Factory() = default;
	virtual AArc<BaseType> createObject() = 0;

	AArc<BaseType> operator()()
	{
		return createObject();
	}

	template <typename DerivedType>
	static AArc<Factory<BaseType>> makeFactoryOf() {
		class MyFactory: public Factory<BaseType>
		{
		public:
			AArc<BaseType> createObject() override
			{
				return _new<DerivedType>();
			}
		};
		return _new<MyFactory>();
	}
};