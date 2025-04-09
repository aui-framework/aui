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

#pragma once

#include <AUI/Common/AString.h>

#ifdef unix
#undef unix
#endif

namespace aui::impl {
    struct Error {
        int nativeCode;
        AString description;
    };

    API_AUI_CORE void lastErrorToException(AString message);
    API_AUI_CORE Error formatSystemError();
    API_AUI_CORE Error formatSystemError(int status);

    namespace unix_based {
        API_AUI_CORE void lastErrorToException(AString message); // unix errors are suitable under windows
        API_AUI_CORE Error formatSystemError();
    }
}

