#pragma once

#include <exception>

#include "AUI/Core.h"

class AObject;

class API_AUI_CORE AAbstractSignal
{
private:
    bool mDestroyed = false;

protected:
	void linkSlot(AObject* object);
	void unlinkSlot(AObject* object);
	
public:
	virtual void clearAllConnectionsWith(AObject* object) = 0;
	virtual void clearAllConnections() = 0;
	virtual ~AAbstractSignal() {
	    mDestroyed = true;
	}

    [[nodiscard]] bool isDestroyed() const {
        return mDestroyed;
    }

    class Disconnect: public std::exception {};
};
