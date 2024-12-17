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

#include <gtest/gtest.h>
#include "AUI/View/ALabel.h"

namespace {
    struct Model {
        AString text;
        int number = 0;
    };
}

TEST(DataBinding, Setter) {
    auto l = _new<ALabel>();
    auto model = _new<ADataBinding<Model>>(Model{
        .text = "text1"
    });

    l && model(&Model::text);

    EXPECT_EQ(l->text(), "text1");

    model->setValue(&Model::text, "text2");

    EXPECT_EQ(l->text(), "text2");
}

TEST(DataBinding, SetterSpecifiedMember) {
    auto l = _new<ALabel>();
    auto model = _new<ADataBinding<Model>>(Model{
        .text = "text1"
    });

    l && model(&Model::text, &ALabel::setText);

    EXPECT_EQ(l->text(), "text1");

    model->setValue(&Model::text, "text2");

    EXPECT_EQ(l->text(), "text2");
}

TEST(DataBinding, SetterCustom) {
    auto l = _new<ALabel>();
    auto model = _new<ADataBinding<Model>>(Model{
            .text = "text1"
    });

    l && model(&Model::text, [](ALabel& label, const AString& s) {
        label.setText("{} custom"_format(s));
    });

    EXPECT_EQ(l->text(), "text1 custom");

    model->setValue(&Model::text, "text2");

    EXPECT_EQ(l->text(), "text2 custom");
}

TEST(DataBinding, SetterProjection) {
    auto l = _new<ALabel>();
    auto model = _new<ADataBinding<Model>>(Model{});

    l && model(&Model::number, AString::number<int>);

    EXPECT_EQ(l->text(), "0");

    model->setValue(&Model::number, 2);

    EXPECT_EQ(l->text(), "2");
}
