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

#include <AUI/Platform/Entry.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/AMimedData.h>
#include "AUI/Common/AByteBuffer.h"

AUI_ENTRY {
    ALogger::info("Hello world!");
    AMimedData d;

    auto c = AString::fromUtf8(AByteBuffer::fromStream(":test.txt"_url.open()));
    ALogger::info("test.txt") << c;
    return c == "test" ? 0 : -1;
}
