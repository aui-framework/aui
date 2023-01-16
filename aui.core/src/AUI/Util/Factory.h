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

#include <AUI/Common/SharedPtr.h>

template <typename BaseType>
class Factory
{
public:
	virtual ~Factory() = default;
	virtual _<BaseType> createObject() = 0;

	_<BaseType> operator()()
	{
		return createObject();
	}

	template <typename DerivedType>
	static _<Factory<BaseType>> makeFactoryOf() {
		class MyFactory: public Factory<BaseType>
		{
		public:
			_<BaseType> createObject() override
			{
				return _new<DerivedType>();
			}
		};
		return _new<MyFactory>();
	}
};