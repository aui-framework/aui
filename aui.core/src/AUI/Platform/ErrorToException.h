// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

