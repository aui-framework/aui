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