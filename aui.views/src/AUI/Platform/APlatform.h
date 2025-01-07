/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "AUI/Views.h"

/**
 * @defgroup windows_specific Windows-specific
 * @ingroup core
 * @details
 * Classes and functions specific to Windows platform.
 */

class AString;

/**
 * @brief System-specific functions.
 */
namespace APlatform
{
    API_AUI_VIEWS AString getFontPath(const AString& font);
    API_AUI_VIEWS void openUrl(const AUrl& url);
    API_AUI_VIEWS float getDpiRatio();
};
