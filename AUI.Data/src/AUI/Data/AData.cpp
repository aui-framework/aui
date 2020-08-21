#include "AData.h"
#include "AUI/Autumn/Autumn.h"

void AData::setDatabase(_<ASqlDatabase> db)
{
	Autumn::put(db);
}
