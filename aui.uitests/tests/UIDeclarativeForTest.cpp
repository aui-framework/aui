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

#include <gmock/gmock.h>
#include "AUI/UITest.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/Model/AListModel.h"

class UIDeclarativeForTest: public testing::UITest {
public:
    ~UIDeclarativeForTest() override = default;

protected:
    void SetUp() override {
        UITest::SetUp();
        mWindow = _new<AWindow>();
        mWindow->show();
    }
    _<AWindow> mWindow;
};

using namespace declarative;
using namespace ass;

struct Item {
    AString text;
};

TEST_F(UIDeclarativeForTest, Basic) {
    auto model = AListModel<Item>::make({
      { .text = "Item 1" },
      { .text = "Item 2" },
      { .text = "Item 3" },
    });

    mWindow->setContents(Vertical {
      AScrollArea::Builder().withContents(AUI_DECLARATIVE_FOR(i, model, AVerticalLayout) {
          return Label { i.text };
      }).build() with_style { FixedSize { 150_dp, 200_dp } },
    });

    uitest::frame();

    EXPECT_TRUE(By::text("Item 1").one());
    EXPECT_TRUE(By::text("Item 2").one());
    EXPECT_TRUE(By::text("Item 3").one());
    EXPECT_FALSE(By::text("Item 4").one());
}

TEST_F(UIDeclarativeForTest, Performance) {
    ::testing::GTEST_FLAG(throw_on_failure) = true;

    class ListModel: public IListModel<Item> {
    public:
        ListModel() {
            ON_CALL(*this, listItemAt(testing::_)).WillByDefault([](const AListModelIndex& i) {
                return Item { "Item {}"_format(i.getRow()) };
            });
        }

        size_t listSize() override { return 1000000; }

        MOCK_METHOD(Item, listItemAt, (const AListModelIndex& i), (override));
    };


    auto model = _new<ListModel>();
    EXPECT_CALL(*model, listItemAt(testing::_)).Times(testing::Between(10, 30));

    mWindow->setContents(Vertical {
        AScrollArea::Builder().withContents(AUI_DECLARATIVE_FOR(i, model, AVerticalLayout) {
          return Label { i.text };
        }).build() with_style { FixedSize { 150_dp, 200_dp } },
    });

    uitest::frame();

    EXPECT_TRUE(By::text("Item 1").one());
    EXPECT_TRUE(By::text("Item 2").one());
    EXPECT_TRUE(By::text("Item 3").one());
    EXPECT_FALSE(By::text("Item 979878").one());
}
