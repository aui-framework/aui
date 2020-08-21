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
	for (auto& a : mSignals)
	{
		a->onObjectHasDestroyed(this);
	}
}

const _<AAbstractThread>& AObject::thread() const
{
	return mAttachedThread;
}
