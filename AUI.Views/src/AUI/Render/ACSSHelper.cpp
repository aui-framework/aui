#include "ACSSHelper.h"

void ACSSHelper::onInvalidateViewCss()
{
	emit invalidateViewCss;
}

void ACSSHelper::onCheckPossiblyMatchCss()
{
	emit checkPossiblyMatchCss;
}
