#pragma once

#include "AUI/Common/AString.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/SharedPtr.h"

template<typename T, typename Container>
class Cache
{
private:
	AMap<AString, _<T>> mContainer;
	
public:
	virtual _<T> load(const AString& key) = 0;
	
	static _<T> get(const AString& key)
	{
		if (auto i = Container::inst().mContainer.contains(key))
		{
			return i->second;
		}
		auto value = Container::inst().load(key);
		put(key, value);
		return value;
	}
	
	static void put(const AString& key, _<T> value)
	{
        Container::inst().mContainer[key] = value;
	}

    static void cleanup() {
        Container::inst(). mContainer.clear();
    }
};
