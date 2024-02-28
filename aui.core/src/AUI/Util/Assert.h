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

#include <cassert>

namespace aui::assertion::detail {
    inline void checkArgs(bool cond, const char* what = nullptr) {}
}

#define AUI_ASSERT(cond) assert(cond); aui::assertion::detail::checkArgs(cond)
#define AUI_ASSERTX(cond, what) assert((cond) && what); aui::assertion::detail::checkArgs(cond, what)