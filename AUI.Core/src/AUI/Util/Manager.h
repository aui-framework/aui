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



