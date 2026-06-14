
/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "AMetric.h"

#include "AUI/IO/AStringStream.h"
#include "AUI/Util/ATokenizer.h"
#include "AUI/Common/AMap.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/APlatform.h"

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
    if (AWindow::current()) {
        dpi = AWindow::current()->getDpiRatio();
    } else {
        dpi = 1.0f;
    }

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
    if (AWindow::current()) {
        dpi = AWindow::current()->getDpiRatio();
    } else {
        dpi = 1.0f;
    }

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
    if (AWindow::current()) {
        dpi = AWindow::current()->getDpiRatio();
    } else {
        dpi = 1.0f;
    }

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
