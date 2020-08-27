#pragma once
#include "AObject.h"
#include "ASignal.h"

class API_AUI_CORE ATimer: public AObject
{
private:
	_<AThread> mThread;
	bool mResetFlag = false;
	unsigned mMsPeriod;
	
public:
	explicit ATimer(unsigned msPeriod);
	virtual ~ATimer();
	
	void restart();

	void start();
	void stop();
	void setPeriodMs(unsigned period) {
	    mMsPeriod = period;
	}
	bool isStarted();

	signals:
	emits<> fired;
};
