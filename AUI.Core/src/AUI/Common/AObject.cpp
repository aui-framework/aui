#include "AObject.h"
#include "AAbstractSignal.h"
#include "AUI/Thread/AThread.h"


void AObject::disconnect()
{
	throw AAbstractSignal::Disconnect();
}

AObject::AObject()
{
	mAttachedThread = AThread::current();
}

AObject::~AObject()
{
	clearSignals();
}

void AObject::clearSignals()
{
    std::unique_lock lock(mSignalsLock);
	for (auto& a : mSignals)
	{
	    if (!a->isDestroyed())
		    a->onObjectHasDestroyed(this);
	}
}
