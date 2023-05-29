
// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "AMetric.h"

#include "AUI/IO/AStringStream.h"
#include "AUI/Util/ATokenizer.h"
#include "AUI/Common/AMap.h"
#include "AUI/Render/Render.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/APlatform.h"

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
        dpi = APlatform::getDpiRatio();

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
        dpi = APlatform::getDpiRatio();

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
        dpi = APlatform::getDpiRatio();

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
