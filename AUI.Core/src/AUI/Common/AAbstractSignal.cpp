#include "AAbstractSignal.h"
#include "AObject.h"

void AAbstractSignal::linkSlot(AObject* object)
{
    std::unique_lock lock(object->mSignalsLock);
	object->mSignals.insert(this);
}

void AAbstractSignal::unlinkSlot(AObject* object)
{
    std::unique_lock lock(object->mSignalsLock);
	object->mSignals.erase(this);
}
