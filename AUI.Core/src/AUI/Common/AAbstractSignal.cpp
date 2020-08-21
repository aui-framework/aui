#include "AAbstractSignal.h"
#include "AObject.h"

void AAbstractSignal::linkSlot(AObject* object)
{
	object->mSignals.insert(this);
}

void AAbstractSignal::unlinkSlot(AObject* object)
{
	object->mSignals.erase(this);
}
