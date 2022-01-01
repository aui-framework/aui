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

#include <boost/test/unit_test.hpp>
#include <AUI/Model/AModels.h>
#include <AUI/Model/AListModel.h>

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(Models)


_<IListModel<int>> testModel() {
    return AListModel<int>::make({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
}

BOOST_AUTO_TEST_CASE(RangesIncluding) {
    auto model = testModel();

    auto check = [](const AVector<AModelRange<int>>& v1, const AVector<AModelRange<int>>& v2) {
        BOOST_REQUIRE_EQUAL(v1, v2);
    };

    check(model->rangesIncluding([&](size_t i) {
        return i < 5;
    }), { model->range(0, 5) });

    check(model->rangesIncluding([&](size_t i) {
        return i < 5 || i >= 8;
    }), { model->range(0, 5), model->range(8, 10) });
    check(model->rangesIncluding([&](size_t i) {
        return i % 2 == 0;
    }), { model->range(0),
          model->range(2),
          model->range(4),
          model->range(6),
          model->range(8), });
}

BOOST_AUTO_TEST_SUITE_END()