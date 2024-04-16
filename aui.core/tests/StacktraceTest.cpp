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

//
// Created by alex2 on 30.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/Util.h>
#include <random>
#include <ctime>
#include "AUI/Common/ATimer.h"
#include "AUI/Logging/ALogger.h"

void someFunction() {
    throw AException("my exception");
}


TEST(Exception, Stacktrace) {
    try {
        someFunction();
        FAIL() << "exception not thrown";
    } catch (const AException& e) {
        std::stringstream ss;
        ss << e;
        AString s = ss.str();
        EXPECT_TRUE(s.contains("my exception"));
        EXPECT_TRUE(s.contains("AException"));
        EXPECT_TRUE(s.contains("- at"));
    }
}
