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

//
// Created by alex2 on 30.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Common/AVector.h>
#include "AUI/Util/APimpl.h"
#include "AUI/Traits/values.h"
#include "AUI/Common/AException.h"
#include "AUI/Thread/AFuture.h"

TEST(Pimpl, Test1) {

    struct MyClass;
    AOptional<aui::fast_pimpl<MyClass, 8>> pimpl;

    struct MyClass {
        int field = 2;
    };
    pimpl = MyClass();

    EXPECT_EQ((*pimpl)->field, 2);
}

TEST(Pimpl, DestructorTest) {
    thread_local bool destructorCalled = false;

    struct MyClass;
    AOptional<aui::fast_pimpl<MyClass, 8>> pimpl;

    struct MyClass {
        int field = 2;

        ~MyClass() {
            destructorCalled = true;
        }
    };

    pimpl = MyClass();
    pimpl.reset();

    EXPECT_TRUE(destructorCalled);
}
