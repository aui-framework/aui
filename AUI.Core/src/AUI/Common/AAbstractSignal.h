#pragma once

#include <exception>

#include "AUI/Core.h"

class AObject;

class API_AUI_CORE AAbstractSignal
{
protected:
	void linkSlot(AObject* object);
	void unlinkSlot(AObject* object);
	
public:
	virtual void onObjectHasDestroyed(AObject* object) = 0;
	virtual ~AAbstractSignal() = default;

	class Disconnect: public std::exception {};
};
