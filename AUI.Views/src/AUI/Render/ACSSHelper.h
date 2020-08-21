#pragma once
#include "AUI/Views.h"
#include "AUI/Common/AObject.h"
#include "AUI/Common/ASignal.h"

class API_AUI_VIEWS ACSSHelper: public AObject
{
public:
	void onInvalidateViewCss();
	void onCheckPossiblyMatchCss();
	
signals:
	emits<> invalidateViewCss;
	emits<> checkPossiblyMatchCss;
};
