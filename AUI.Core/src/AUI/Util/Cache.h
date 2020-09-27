#pragma once

#include "AUI/Common/AString.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/SharedPtr.h"
#include "Singleton.h"

template<typename T, typename Container>
class Cache: Singleton<Container>
{
private:
	AMap<AString, _<T>> mContainer;
	
public:
	virtual _<T> load(const AString& key) = 0;
	
	static _<T> get(const AString& key)
	{
		if (auto i = Singleton<Container>::instance().mContainer.contains(key))
		{
			return i->second;
		}
		auto value = Singleton<Container>::instance().load(key);
		put(key, value);
		return value;
	}
	
	static void put(const AString& key, _<T> value)
	{
		Singleton<Container>::instance().mContainer[key] = value;
	}

    static void cleanup() {
        Singleton<Container>::instance(). mContainer.clear();
    }
};
