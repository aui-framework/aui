/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "AMetric.h"

#include "AUI/IO/AStringStream.h"
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
	ATokenizer p(_new<AStringStream>(text));
	mValue = p.readFloat();

	auto unitName = p.readString();

	if (unitName.empty())
	{
		mUnit = T_PX;
	}
	else {
		AMap<AString, Unit> unitMapping = {
			{ "px", T_PX },
			{ "em", T_DP },
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
	case T_DP:
		return mValue * dpi;
	case T_PT:
		return mValue * dpi / 3.f * 4.f;
		
	default:
		return mValue;
	}
}

float AMetric::fromPxToMetric(float value, AMetric::Unit unit) {
    float dpi;
    if (AWindow::current())
        dpi = AWindow::current()->getDpiRatio();
    else
        dpi = Platform::getDpiRatio();

    switch (unit)
    {
        case T_PX:
            return value;
        case T_DP:
            return value / dpi;
        case T_PT:
            return value / dpi * 3.f / 4.f;

        default:
            return value;
    }
}

float AMetric::getValueDp() const {
    float dpi;
    if (AWindow::current())
        dpi = AWindow::current()->getDpiRatio();
    else
        dpi = Platform::getDpiRatio();

    switch (mUnit)
    {
        case T_PX:
            return mValue / dpi;
        case T_DP:
            return mValue;
        case T_PT:
            return mValue * 3.f / 4.f;

        default:
            return mValue;
    }
}
