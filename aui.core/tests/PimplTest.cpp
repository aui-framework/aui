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
