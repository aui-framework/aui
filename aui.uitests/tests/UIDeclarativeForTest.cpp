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
#include <range/v3/all.hpp>
#include <AUI/View/AForEachUI.h>
#include "AUI/UITest.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/Model/AListModel.h"

static constexpr auto LOG_TAG = "UIDeclarativeForTest";

namespace {
    class Observer {
    public:
        MOCK_METHOD(void, onViewCreated, (const AString &s), ());
    };
}   // namespace

class UIDeclarativeForTest : public testing::UITest {
public:
    ~UIDeclarativeForTest() override = default;

protected:
    void SetUp() override {
        UITest::SetUp();

        ON_CALL(mTestObserver, onViewCreated(testing::_)).WillByDefault([](const AString &s) {
            ALOG_DEBUG(LOG_TAG) << "View instantiated: " << s;
        });

        mWindow = _new<AWindow>();
        mWindow->show();
    }

    _<AWindow> mWindow;
    Observer mTestObserver;

    void validateOrder() {
        uitest::frame();
        auto labels = By::type<ALabel>().toVector();
        ASSERT_FALSE(labels.empty());

        AOptional<int> lastGroup;
        AOptional<int> lastItem;

        for (const auto &view : labels) {
            auto text = *_cast<ALabel>(view)->text();
            if (text.startsWith("Group")) {
                auto currentGroup = text.substr(6).toIntOrException();
                if (lastGroup) {
                    EXPECT_LT(*lastGroup, currentGroup);
                }
                lastGroup = currentGroup;
                continue;
            }
            auto currentItem = text.toIntOrException();
            if (lastItem) {
                EXPECT_LT(*lastItem, currentItem);
                if (lastGroup) {
                    EXPECT_LE(*lastGroup, currentItem);
                }
            }
            lastItem = currentItem;
        }
    }
};

using namespace declarative;
using namespace ass;

struct Item {
    AString text;
};
/*
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
}*/

TEST_F(UIDeclarativeForTest, Performance) {
    ::testing::GTEST_FLAG(throw_on_failure) = true;

    EXPECT_CALL(mTestObserver, onViewCreated(testing::_)).Times(testing::Between(10, 40));

    mWindow->setContents(Vertical {
      AScrollArea::Builder()
              .withContents(
              AUI_DECLARATIVE_FOR_EX(i, ranges::views::ints, AVerticalLayout, &) {
                  mTestObserver.onViewCreated("");
                  return Label { "Item {}"_format(i) };
              })
              .build() with_style { FixedSize { 150_dp, 200_dp } },
    });

    uitest::frame();

    EXPECT_TRUE(By::text("Item 1").one());
    EXPECT_TRUE(By::text("Item 2").one());
    EXPECT_TRUE(By::text("Item 3").one());
    EXPECT_FALSE(By::text("Item 979878").one());
}

TEST_F(UIDeclarativeForTest, Dynamic) {
    AProperty<AVector<AString>> items = AVector<AString> { "Hello", "World", "Test" };
    mWindow->setContents(Vertical {
      AScrollArea::Builder().withContents(AUI_DECLARATIVE_FOR_EX(i, *items, AVerticalLayout, &) { return Label { i }; }).build() with_style { FixedSize { 150_dp, 200_dp } },
    });

    auto checkAllPresent = [&] {
        uitest::frame();
        for (const auto &i : *items) {
            EXPECT_TRUE(By::text(i).one()) << i;
        }
    };

    checkAllPresent();
    items.writeScope()->push_back("Bruh");
    checkAllPresent();
}

TEST_F(UIDeclarativeForTest, DynamicPerformance) {
    AProperty<AVector<AString>> items = AVector<AString> { "Hello", "World", "Test" };

    ::testing::GTEST_FLAG(throw_on_failure) = true;

    testing::InSequence s;
    EXPECT_CALL(mTestObserver, onViewCreated("Hello"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("World"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("Test"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("Bruh"_as));

    mWindow->setContents(Vertical {
      AScrollArea::Builder()
              .withContents(
              AUI_DECLARATIVE_FOR_EX(i, *items, AVerticalLayout, &) {
                  mTestObserver.onViewCreated(i);
                  return Label { i };
              })
              .build() with_style { FixedSize { 150_dp, 200_dp } },
    });

    uitest::frame();
    items.writeScope()->push_back("Bruh");
    uitest::frame();
}

TEST_F(UIDeclarativeForTest, IntBasic) {
    mWindow->setContents(Vertical {
      AScrollArea::Builder()
              .withContents(
              AUI_DECLARATIVE_FOR(i, ranges::views::ints, AVerticalLayout) { return Label { "{}"_format(i) }; })
              .build() with_style { FixedSize { 150_dp, 200_dp } },
    });

    validateOrder();
}

TEST_F(UIDeclarativeForTest, IntBasic2) {
    AVector<int> mInts = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    mWindow->setContents(Vertical {
      AScrollArea::Builder()
              .withContents(
              AUI_DECLARATIVE_FOR(i, mInts, AVerticalLayout) { return Label { "{}"_format(i) }; })
              .build() with_style { FixedSize { 150_dp, 200_dp } },
    });

    validateOrder();
}

TEST_F(UIDeclarativeForTest, IntGrouping) {
    AVector<int> mInts = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

    mWindow->setContents(Vertical {
      AScrollArea::Builder()
              .withContents(
              AUI_DECLARATIVE_FOR(
                  group, mInts | ranges::views::chunk_by([](int l, int r) { return l / 10 == r / 10; }),
                  AVerticalLayout) {
                  return Vertical {
                      Label { "Group {}"_format(*ranges::begin(group) / 10 * 10) } with_style { FontSize(10_pt) },
                      AUI_DECLARATIVE_FOR(i, group, AVerticalLayout) { return Label { "{}"_format(i) }; }
                  };
              })
              .build() with_style { FixedSize { 150_dp, 200_dp } },
    });

    validateOrder();
    EXPECT_TRUE(By::text("Group 0").one());
    EXPECT_TRUE(By::text("Group 10").one());
}

TEST_F(UIDeclarativeForTest, IntGroupingDynamic1) {
    ::testing::GTEST_FLAG(throw_on_failure) = true;
    AProperty<AVector<int>> mInts = AVector<int> { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

    testing::InSequence s;

    mWindow->setContents(Vertical {
      AScrollArea::Builder()
              .withContents(
                  /* group foreach */
              AUI_DECLARATIVE_FOR(
                  group, *mInts | ranges::views::chunk_by([](int l, int r) { return l / 10 == r / 10; }),
                  AVerticalLayout) {
                  /* group foreach data to view transformer callback, aka group's view callback */
                  auto groupName = "Group {}"_format(*ranges::begin(group) / 10 * 10);
                  mTestObserver.onViewCreated(groupName);
                  return Vertical {
                      Label { groupName } with_style { FontSize(10_pt) },
                      /* single item foreach */
                      AUI_DECLARATIVE_FOR(i, group, AVerticalLayout) {
                          /* single item data to view transformer callback, aka item's view callback */
                          auto str = "{}"_format(i);
                          mTestObserver.onViewCreated(str);
                          return Label { std::move(str) };
                      }
                  };
              })
              .build() with_style { FixedSize { 150_dp, 200_dp } },
    });

    EXPECT_CALL(mTestObserver, onViewCreated("Group 0"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("1"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("2"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("3"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("4"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("5"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("6"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("7"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("8"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("9"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("Group 10"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("10"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("11"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("12"_as));

    validateOrder();
    EXPECT_TRUE(By::text("Group 0").one());
    EXPECT_TRUE(By::text("Group 10").one());

    /* basic removal */
    /* An attempt to change the first chunk will re-evaluate group's view callback for "Group 0". */
    EXPECT_CALL(mTestObserver, onViewCreated("Group 0"_as));
    mInts.writeScope()->removeAll(2);
    validateOrder();
    /* Despite that, in fact, a new single item foreach (of "Group 0") is created, its callback won't be reevaluated for
     * each single item ("0", "1", "2", ...). There's a cache of instantiated views, denoted by callback's type. This
     * cache is shared within one C++'s compilation unit (a cpp file). */

    /* update & reorder to existing group */
    {
        auto write = mInts.writeScope();
        write->removeAll(3);
        write->push_back(15);
    }
    validateOrder();
    EXPECT_FALSE(By::text("3").one());
    EXPECT_TRUE(By::text("15").one());

    /* update & reorder to new group */
    {
        auto write = mInts.writeScope();
        write->removeAll(4);
        write->push_back(22);
    }
    validateOrder();
    EXPECT_FALSE(By::text("4").one());
    EXPECT_TRUE(By::text("Group 20").one());
    EXPECT_TRUE(By::text("22").one());

    /* remove first element from the group */
    mInts.writeScope()->removeAll(1);
}
