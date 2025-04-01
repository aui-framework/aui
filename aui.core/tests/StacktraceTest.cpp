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
