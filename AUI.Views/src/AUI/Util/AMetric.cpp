#include "AMetric.h"

#include "AUI/IO/StringStream.h"
#include "AUI/Util/ATokenizer.h"
#include "AUI/Common/AMap.h"
#include "AUI/Render/Render.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/Platform.h"

AMetric::AMetric(float value, Unit unit): mValue(value),
                                          mUnit(unit)
{
}

AMetric::AMetric(const AString& text)
{
	ATokenizer p(_new<StringStream>(text));
	mValue = p.readFloat();

	auto unitName = p.readString();

	if (unitName.empty())
	{
		mUnit = T_PX;
	}
	else {
		AMap<AString, Unit> unitMapping = {
			{ "px", T_PX },
			{ "em", T_EM },
			{ "pt", T_PT },
		};

		if (auto x = unitMapping.contains(unitName))
		{
			mUnit = x->second;
		}
	}
	
}

float AMetric::getValuePx() const
{
	float dpi;
	if (AWindow::current())
		dpi = AWindow::current()->getDpiRatio();
	else
		dpi = Platform::getDpiRatio();
	
	switch (mUnit)
	{
	case T_PX:
		return mValue;
	case T_EM:
		return mValue * dpi;
	case T_PT:
		return mValue * dpi / 3.f * 4.f;
		
	default:
		return mValue;
	}
}
