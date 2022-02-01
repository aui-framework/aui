/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 31.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Traits/values.h>

TEST(Lazy, Unused) {
    aui::lazy<int> unused = [] {
        EXPECT_TRUE(false) << "initializer function called";
        return 0;
    };
}
TEST(Lazy, Used) {
    bool called = false;
    aui::lazy<int> used = [&] {
        EXPECT_FALSE(called) << "initializer function called twice";
        called = true;
        return 123;
    };
    ASSERT_FALSE(called);
    ASSERT_EQ(int(used), 123);
    ASSERT_TRUE(called);
    ASSERT_EQ(*used, 123);
    ASSERT_EQ(used, 123);

    ASSERT_EQ(int((const aui::lazy<int>)used), 123);
    ASSERT_EQ(*((const aui::lazy<int>)used), 123);
    ASSERT_EQ(((const aui::lazy<int>)used), 123);

    ASSERT_TRUE(called);
}
