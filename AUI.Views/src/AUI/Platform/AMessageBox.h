#pragma once
#include "AUI/Util/EnumUtil.h"
#include "AUI/Views.h"


class AString;
class AWindow;

namespace AMessageBox
{
	enum Icon
	{
		I_NONE,
		I_INFO,
		I_WARNING,
		I_CRITICAL
	};
	ENUM_FLAG(Button)
	{
		B_OK = 0x1,
		B_CANCEL = 0x2,
        B_INVALID = -1,
	};
	API_AUI_VIEWS Button show(AWindow* parent, const AString& title, const AString& message, Icon icon = I_NONE, Button b = B_OK);
};
