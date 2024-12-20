﻿/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "AUI/Platform/APlatform.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/APath.h"
#include <AUI/Util/kAUI.h>
#include <AUI/Platform/android/OSAndroid.h>



float APlatform::getDpiRatio()
{
    return com::github::aui::android::Platform::getDpiRatio();
}

void APlatform::openUrl(const AUrl &url) {
    com::github::aui::android::Platform::openUrl(url.full());
}
