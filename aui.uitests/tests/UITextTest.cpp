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

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AText.h>
#include <AUI/View/AScrollArea.h>

using namespace ass;
using namespace declarative;


class UIText: public testing::UITest {
public:
    ~UIText() override = default;

protected:
    void SetUp() override {
        UITest::SetUp();


        auto window = _new<AWindow>();
        window->setContents(Centered {
            mText
        });
        window->pack();
        window->show();
        uitest::frame();
    }

    _<AText> mText = AText::fromString("Привет мир\nвторая строка");


    void TearDown() override {
        UITest::TearDown();
    }
};


TEST_F(UIText, Contents) {
    auto& entries = mText->mEngine.entries();
    ASSERT_EQ(entries.size(), 7);
    EXPECT_EQ(AString::fromUtf32(_cast<AText::WordEntry>(entries[0])->getWord()), "Привет");
    EXPECT_NE(_cast<aui::detail::WhitespaceEntry>(entries[1]), nullptr);
    EXPECT_EQ(AString::fromUtf32(_cast<AText::WordEntry>(entries[2])->getWord()), "мир");
    EXPECT_NE(_cast<aui::detail::NextLineEntry>(entries[3]), nullptr);
    EXPECT_EQ(AString::fromUtf32(_cast<aui::detail::WordEntry>(entries[4])->getWord()), "вторая");
    EXPECT_NE(_cast<aui::detail::WhitespaceEntry>(entries[5]), nullptr);
    EXPECT_EQ(AString::fromUtf32(_cast<aui::detail::WordEntry>(entries[6])->getWord()), "строка");
}
