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

//
// Created by nelonn on 9/12/25.
//

#pragma once

#include <AUI/Common/AString.h>

class APathView: public AStringView {
private:
#if AUI_PLATFORM_WIN
    struct _stat64 stat() const;
#else
    struct stat stat() const;
#endif


};
