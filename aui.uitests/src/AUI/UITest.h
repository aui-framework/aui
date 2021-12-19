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

#pragma once


#include <AUI/View/AView.h>

namespace UITest {
    API_AUI_UITESTS void frame();
}
#include <boost/test/unit_test.hpp>
#include <AUI/Test/UI/Matcher.h>
#include <AUI/Test/UI/By.h>
#include <AUI/Test/UI/UITestCase.h>

#include <AUI/Test/UI/Action/Click.h>
#include <AUI/Test/UI/Action/Type.h>

#include <AUI/Test/UI/Assertion/Empty.h>
#include <AUI/Test/UI/Assertion/Gone.h>
#include <AUI/Test/UI/Assertion/Visible.h>
#include <AUI/Test/UI/Assertion/NotVisible.h>
#include <AUI/Test/UI/Assertion/Text.h>
