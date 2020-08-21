#pragma once

#include "AUI/Core.h"

template <typename T>
class Singleton {
public:
	Singleton() {}
	virtual ~Singleton() {}
	static T& instance() {
		static T t;
		return t;
	}
};