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

#include <gtest/gtest.h>
#include "Formula.h"
#include "Spreadsheet.h"

class Cells_Formula: public testing::Test {
public:

protected:
    Spreadsheet mSpreadsheet{glm::uvec2(100)};
};

TEST_F(Cells_Formula, Constant) {
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "0")), 0.0);
}

TEST_F(Cells_Formula, String) {
    EXPECT_EQ(std::get<AString>(formula::evaluate(mSpreadsheet, "test")), "test");
}

TEST_F(Cells_Formula, EConstant) {
    EXPECT_DOUBLE_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=1")), 1.0);
}

TEST_F(Cells_Formula, Math1) {
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=1+2")), 3);
}

TEST_F(Cells_Formula, Math2) {
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=5/2")), 2.5f);
}

TEST_F(Cells_Formula, Math3) {
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=(2+3)*4")), 20);
}

TEST_F(Cells_Formula, Math4) {
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=4*(2+3)")), 20);
}

TEST_F(Cells_Formula, Math5) {
    mSpreadsheet[{0, 0}].expression = "1";
    mSpreadsheet[{1, 0}].expression = "2";
    mSpreadsheet[{2, 0}].expression = "3";
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=A0+B0*C0")), 7);
}

TEST_F(Cells_Formula, Math6) {
    mSpreadsheet[{0, 0}].expression = "1";
    mSpreadsheet[{1, 0}].expression = "2";
    mSpreadsheet[{2, 0}].expression = "3";
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=(A0+B0)*C0")), 9);
}

TEST_F(Cells_Formula, FSum1) {
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=SUM(1;2)")), 3);
}

TEST_F(Cells_Formula, FSum2) {
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=SUM(1;2;3)")), 6);
}

TEST_F(Cells_Formula, CellRef) {
    mSpreadsheet[{0, 0}].expression = "228";
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=A0")), 228);

    mSpreadsheet[{0, 0}].expression = "229";
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=A0")), 229);
}

TEST_F(Cells_Formula, ChangePropagation) {
    mSpreadsheet[{1, 0}].expression = "=A0+1";

    mSpreadsheet[{0, 0}].expression = "228";
    EXPECT_EQ(std::get<double>(mSpreadsheet[{1, 0}].value.value()), 229);

    mSpreadsheet[{0, 0}].expression = "0";
    EXPECT_EQ(std::get<double>(mSpreadsheet[{1, 0}].value.value()), 1);
}

TEST_F(Cells_Formula, Range1) {
    mSpreadsheet[{0, 0}].expression = "1";
    mSpreadsheet[{0, 1}].expression = "2";
    mSpreadsheet[{0, 2}].expression = "3";
    mSpreadsheet[{1, 0}].expression = "3";
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=SUM(A:A)")), 6);
}

TEST_F(Cells_Formula, Range2) {
    mSpreadsheet[{0, 0}].expression = "1";
    mSpreadsheet[{0, 1}].expression = "2";
    mSpreadsheet[{0, 2}].expression = "3";
    mSpreadsheet[{1, 0}].expression = "4";
    mSpreadsheet[{1, 1}].expression = "5";
    mSpreadsheet[{1, 2}].expression = "6";
    mSpreadsheet[{2, 0}].expression = "7";
    mSpreadsheet[{2, 1}].expression = "8";
    mSpreadsheet[{2, 2}].expression = "9";
    // 1 4 7
    // 2 5 8
    // 3 6 9
    EXPECT_EQ(std::get<double>(formula::evaluate(mSpreadsheet, "=SUM(A0:B1)")), 12);
}


TEST_F(Cells_Formula, EvaluationLoop) {
    mSpreadsheet[{ 0, 0 }].expression = "=A1";
    mSpreadsheet[{ 0, 1 }].expression = "=A0";
    EXPECT_EQ(std::get<AString>(*mSpreadsheet[{ 0, 1 }].value), "#LOOP!");
}
