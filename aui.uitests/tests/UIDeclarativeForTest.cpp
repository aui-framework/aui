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

#include <gmock/gmock.h>
#include <range/v3/all.hpp>
#include <AUI/View/AForEachUI.h>
#include "AUI/UITest.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/Model/AListModel.h"
#include "AUI/View/AButton.h"

static constexpr auto LOG_TAG = "UIDeclarativeForTest";

// clang-format off

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

    template<typename T>
    static const auto& cache() {
        return *_cast<T>(By::type<T>().one())->mViewsSharedCache;
    }
};

using namespace declarative;
using namespace ass;

struct Item {
    AString text;
};

TEST_F(UIDeclarativeForTest, Example) {
    static const std::array users = { "Foo", "Bar", "Lol" };
    mWindow->setContents(Centered {
        AScrollArea::Builder().withContents(
            AUI_DECLARATIVE_FOR(user, users, AVerticalLayout) {
                return Label { fmt::format("{}", user) };
            }
        ).build() AUI_WITH_STYLE { FixedSize { 150_dp, 200_dp } },
    });
    uitest::frame();
    EXPECT_TRUE(By::text("Foo").one());
    EXPECT_TRUE(By::text("Bar").one());
    EXPECT_TRUE(By::text("Lol").one());
    saveScreenshot("");
}


// AUI_DOCS_OUTPUT: doxygen/intermediate/foreach.h
// @class AForEachUI

TEST_F(UIDeclarativeForTest, Performance) {
    ::testing::GTEST_FLAG(throw_on_failure) = true;

    EXPECT_CALL(mTestObserver, onViewCreated(testing::_)).Times(testing::Between(10, 40));

    mWindow-> // HIDE
    setContents(Vertical {
      AScrollArea::Builder()
              .withContents(
              AUI_DECLARATIVE_FOR_EX(i, ranges::views::ints, AVerticalLayout, &) {
                  mTestObserver.onViewCreated("");
                  return Label { "Item {}"_format(i) };
              })
              .build() AUI_WITH_STYLE { FixedSize { 150_dp, 200_dp } },
    });
    // A static range like in the example above will not blow up the machine because AForEachUI is wrapped with a
    // [AScrollArea], thus it is not required to evaluate the whole range, which is infinite in our case.
    //
    // An attempt to update AForEachUI with a large range still can lead to high resource consumption, due to need of
    // recomposition.

    uitest::frame();

    EXPECT_TRUE(By::text("Item 1").one());
    EXPECT_TRUE(By::text("Item 2").one());
    EXPECT_TRUE(By::text("Item 3").one());
    EXPECT_FALSE(By::text("Item 979878").one());
    EXPECT_EQ(cache<AForEachUI<int>>().size(), 0);
}

/**********************************************************************************************************************/
//
// ## Initialization { #AFOREACHUI_UPDATE }
//
// This section explains how to initialize [AUI_DECLARATIVE_FOR], manage lifetime of containers and how to make them
// reactive.
//
// In [AUI_DECLARATIVE_FOR], a potentially [reactive](aui::react) expression evaluating to *range* and the lambda
// that creates a new views are both lambdas with capture default by value `[=]`. This means that:
//
// 1. All mentioned *local* variables are captured by copying.
// 2. All mentioned class member variables (fields) are captured by reference.
//
// Both lambdas can be evaluated at any point during lifetime of a AForEachUI, so the by-value capture makes it's hard
// to introduce dangling references, by either copying locals or referencing class members.
//
// Most modern compilers are capable to optimize out copying and initialize \*copied\* locals just in place.
//
// An attempt to go out of the scenarios listed below will likely lead to a `static_assert` with a link to this section.

TEST_F(UIDeclarativeForTest, Constant_global_data) { // HEADER_H3
    // The most straightforward way is using constant global data:
#ifdef AUI_ENTRY
#undef AUI_ENTRY
#endif
#define AUI_ENTRY [&]
    // AUI_DOCS_CODE_BEGIN
    static constexpr auto COLORS = { "Red", "Green", "Blue", "Black", "White" };

    class MyWindow: public AWindow {
    public:
        MyWindow() {
            setContents(Vertical {
                AUI_DECLARATIVE_FOR(i, COLORS, AVerticalLayout) {
                  return Label { "{}"_format(i) };
                }
            });
        }
    };
    AUI_ENTRY {
      auto w = _new<MyWindow>();
      w->show();
      return 0;
    }
        // AUI_DOCS_CODE_END
        ();
    uitest::frame();
    EXPECT_TRUE(By::text("Red").one());
    EXPECT_TRUE(By::text("White").one());
    EXPECT_EQ(cache<AForEachUI<const char*>>().size(), 0);
}

TEST_F(UIDeclarativeForTest, Constant_global_data2) {
    static constexpr auto COLORS = { "Red", "Green", "Blue", "Black", "White" };
    // It's a good idea to wrap AForEachUI with an [AScrollArea].
    // AUI_DOCS_CODE_BEGIN
    mWindow-> // HIDE
    setContents(Vertical {
        AScrollArea::Builder()
            .withContents(
                AUI_DECLARATIVE_FOR(i, COLORS, AVerticalLayout) {
                  return Label { "{}"_format(i) };
                })
            .build() AUI_WITH_STYLE { FixedSize { 150_dp, 200_dp } },
    });
    // AUI_DOCS_CODE_END

    uitest::frame();
    EXPECT_TRUE(By::text("Red").one());
    EXPECT_TRUE(By::text("White").one());

    // ![](imgs/UIDeclarativeForTest.Constant_global_data2_.png)
    saveScreenshot("");

    EXPECT_EQ(cache<AForEachUI<const char*>>().size(), 0);
}

TEST_F(UIDeclarativeForTest, Infinite_ranges_and_views) { // HEADER_H3
    // Most generators, ranges and views are expected to work.
    // AUI_DOCS_CODE_BEGIN
    mWindow-> // HIDE
    setContents(Vertical {
        AScrollArea::Builder().withContents(
            AUI_DECLARATIVE_FOR(i, ranges::views::ints, AVerticalLayout) {
                return Label { "{}"_format(i) };
            }
        ).build() AUI_WITH_STYLE { FixedSize { 150_dp, 200_dp } },
    });
    // AUI_DOCS_CODE_END

    validateOrder();
    // ![](imgs/UIDeclarativeForTest.Infinite_ranges_and_views_.png)
    saveScreenshot("");

    EXPECT_EQ(cache<AForEachUI<int>>().size(), 0);
}

TEST_F(UIDeclarativeForTest, Transferring_ownership_by_copying) { // HEADER_H3
    // When using locals, their immediate values are copied during initialization of AUI_DECLARATIVE_FOR.
    // AUI_DOCS_CODE_BEGIN
    auto items = AVector<AString> { "Hello", "World", "Test" };
    mWindow-> // HIDE
    setContents(Vertical {
        AScrollArea::Builder().withContents(
            AUI_DECLARATIVE_FOR(i, items, AVerticalLayout) {
               return Label { i };
            }
        ).build() AUI_WITH_STYLE { FixedSize { 150_dp, 200_dp } },
    });
    // AUI_DOCS_CODE_END

    uitest::frame();
    for (const auto &i : items) {
        EXPECT_TRUE(By::text(i).one()) << i;
    }

    // As such, an attempt to modify `items` will not reflect on presentation, because it has own copy of `items`.
    // AUI_DOCS_CODE_BEGIN
    items.push_back("Bruh");
    // AUI_DOCS_CODE_END

    EXPECT_FALSE(By::text("Bruh").one());
    EXPECT_EQ(cache<AForEachUI<AString>>().size(), 0);

    // ![](imgs/UIDeclarativeForTest.Transferring_ownership_by_copying_.png)
    saveScreenshot("");
}

TEST_F(UIDeclarativeForTest, Borrowing_constant_containers) {// HEADER_H3
    // If your container lives inside your class, its value is not copied but referenced. To avoid unobserved iterator
    // invalidation and content changes, [AUI_DECLARATIVE_FOR] requires borrowed containers to be constant. There's a
    // compile-time check to verify this requirement that does work in most cases, like this one.
    //
    // AUI_DOCS_CODE_BEGIN
    class MyWindow: public AWindow {
    public:
        MyWindow(AVector<AString> colors): mColors(std::move(colors)) {
            setContents(Vertical {
                AUI_DECLARATIVE_FOR(i, mColors, AVerticalLayout) {
                  return Label { "{}"_format(i) };
                }
            });
        }
    private:
        const AVector<AString> mColors;
    };
    AUI_ENTRY {
         auto w = _new<MyWindow>(AVector<AString>{ "Red", "Green", "Blue", "Black", "White" });
         w->show();
         return 0;
    }
    // AUI_DOCS_CODE_END
        ();

    //
    // ![](imgs/UIDeclarativeForTest.Borrowing_constant_containers_.png)
    uitest::frame();
    saveScreenshot("");
    EXPECT_EQ(cache<AForEachUI<AString>>().size(), 0);

    //
    // Marking the borrowed container as const effectively saves you from unintended borrowed data changes. If you'd
    // like to change the container or view options and [AUI_DECLARATIVE_FOR] to respond to the changes, read the section
    // below.
}

TEST_F(UIDeclarativeForTest, Reactive_lists) { // HEADER_H3
    // The reason why [AUI_DECLARATIVE_FOR] is so restrictive about using borrowed non-const data is because it stores
    // *range*'s iterators under the hood. Various containers have different rules on iterator invalidation, but, since
    // it accepts any type of *range*, we consider using its iterators after a modifying access to the container or a
    // view as unsafe:
    // - visual presentation by [AUI_DECLARATIVE_FOR] might seem unresponsive to borrowed data changes,
    // - may lead to program crash.
    //
    // To address this issue, we provide a convenient [way](property_system) to make iterator invalidation
    // *observable*:
    //
    // - wrap the container with `AProperty`,
    // - dereference `mColors` in [AUI_DECLARATIVE_FOR] clause.
    //
    // AUI_DOCS_CODE_BEGIN
    class MyWindow: public AWindow {
    public:
        MyWindow(AVector<AString> colors): mColors(std::move(colors)) {
            setContents(Vertical {
                _new<AButton>("Add A new color").connect(&AView::clicked, me::addColor),
                AScrollArea::Builder().withContents(
                  AUI_DECLARATIVE_FOR(i, *mColors, AVerticalLayout) {
                    return Label { "{}"_format(i) };
                  }
                ).build() AUI_WITH_STYLE { FixedSize { 150_dp, 200_dp } },
            });
        }
    private:
        AProperty<AVector<AString>> mColors;

        void addColor() {
            mColors.writeScope()->push_back("A new color");
        }
    };
    AUI_ENTRY {
        auto w = _new<MyWindow>(AVector<AString>{ "Red", "Green", "Blue", "Black", "White" });
        w->show();
        return 0;
    }
    // AUI_DOCS_CODE_END
    ();


    //
    // ![](imgs/UIDeclarativeForTest.Reactive_lists_1.png)
    uitest::frame();
    saveScreenshot("1");
    EXPECT_TRUE(By::text("Red").one());
    EXPECT_TRUE(By::text("White").one());
    EXPECT_FALSE(By::text("A new color").one());

    // Upon clicking "Add A new color" button, the "A new color" label will appear in the list.
    By::text("Add A new color").perform(click());
    //
    // ![](imgs/UIDeclarativeForTest.Reactive_lists_2.png)
    uitest::frame();
    saveScreenshot("2");
    EXPECT_TRUE(By::text("A new color").one());
    EXPECT_EQ(cache<AForEachUI<AString>>().size(), 0);
}

TEST_F(UIDeclarativeForTest, DynamicPerformance) {
    struct State {
        AProperty<AVector<AString>> items = AVector<AString> { "Hello", "World", "Test" };
    };
    auto state = _new<State>();

    ::testing::GTEST_FLAG(throw_on_failure) = true;

    testing::InSequence s;
    EXPECT_CALL(mTestObserver, onViewCreated("Hello"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("World"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("Test"_as));
    EXPECT_CALL(mTestObserver, onViewCreated("Bruh"_as));

    mWindow->setContents(Vertical {
      AScrollArea::Builder()
              .withContents(
              AUI_DECLARATIVE_FOR_EX(i, *state->items, AVerticalLayout, &) {
                  mTestObserver.onViewCreated(i);
                  return Label { i };
              })
              .build() AUI_WITH_STYLE { FixedSize { 150_dp, 200_dp } },
    });

    uitest::frame();
    state->items.writeScope()->push_back("Bruh");
    uitest::frame();
    EXPECT_EQ(cache<AForEachUI<AString>>().size(), 0);
}

TEST_F(UIDeclarativeForTest, IntBasic2) {
    const AVector<int> mInts = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    mWindow->setContents(Vertical {
      AScrollArea::Builder()
              .withContents(
              AUI_DECLARATIVE_FOR(i, mInts, AVerticalLayout) { return Label { "{}"_format(i) }; })
              .build() AUI_WITH_STYLE { FixedSize { 150_dp, 200_dp } },
    });

    validateOrder();
}

TEST_F(UIDeclarativeForTest, IntGrouping) {
    const AVector<int> mInts = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

    mWindow->setContents(Vertical {
      AScrollArea::Builder()
              .withContents(
              AUI_DECLARATIVE_FOR(
                  group, mInts | ranges::views::chunk_by([](int l, int r) { return l / 10 == r / 10; }),
                  AVerticalLayout) {
                  return Vertical {
                      Label { "Group {}"_format(*ranges::begin(group) / 10 * 10) } AUI_WITH_STYLE { FontSize{10_pt} },
                      AUI_DECLARATIVE_FOR(i, group, AVerticalLayout) { return Label { "{}"_format(i) }; }
                  };
              })
              .build() AUI_WITH_STYLE { FixedSize { 150_dp, 200_dp } },
    });

    validateOrder();
    EXPECT_TRUE(By::text("Group 0").one());
    EXPECT_TRUE(By::text("Group 10").one());
    EXPECT_EQ(cache<AForEachUI<int>>().size(), 0);
}

TEST_F(UIDeclarativeForTest, IntGroupingDynamic1) {
    ::testing::GTEST_FLAG(throw_on_failure) = true;

    struct State {
        AProperty<AVector<int>> ints = AVector<int> { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    };
    auto state = _new<State>();

    testing::InSequence s;

    mWindow->setContents(Vertical {
      AScrollArea::Builder()
              .withContents(
                  /* group foreach */
                  AUI_DECLARATIVE_FOR(
                  group, *state->ints | ranges::views::chunk_by([](int l, int r) { return l / 10 == r / 10; }),
                  AVerticalLayout) {
                      /* group foreach data to view transformer callback, aka group's view callback */
                      auto groupName = "Group {}"_format(*ranges::begin(group) / 10 * 10);
                      mTestObserver.onViewCreated(groupName);
                      return Vertical {
                          Label { groupName } AUI_WITH_STYLE { FontSize{10_pt} },
                          /* single item foreach */
                          AUI_DECLARATIVE_FOR(i, group, AVerticalLayout) {
                              /* single item data to view transformer callback, aka item's view callback */
                              auto str = "{}"_format(i);
                              mTestObserver.onViewCreated(str);
                              return Label { std::move(str) };
                          }
                      };
                  })
              .build() AUI_WITH_STYLE { FixedSize { 150_dp, 300_dp } },
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
    /* An attempt to change contents of the first chunk will re-evaluate group's view callback for "Group 0". */
    EXPECT_CALL(mTestObserver, onViewCreated("Group 0"_as));
    /* Also, depending on used container's iterator implementation, other groups might be evaluated as well. In our
     * case, we are using AVector, whose iterator is an offset from beginning. Since the offset has changed due to
     * removal, the iterator is considered dirty. This might not be the case for containers whose items are stored in
     * heap, i.e., `std::list`. */
    EXPECT_CALL(mTestObserver, onViewCreated("Group 10"_as));
    {
        state->ints.writeScope()->removeAll(2);
    }

    validateOrder();

    /* Despite that, in fact, a new group foreach (of "Group 0" and potentially others) is created, single item callback
     * ("0", "1", "2", ...) won't be reevaluated . There's a cache of instantiated views, denoted by
     * callback's type. This cache is shared within one C++'s compilation unit (a cpp file). */

    /* update & reorder to existing group */
    EXPECT_CALL(mTestObserver, onViewCreated("Group 0"_as));  /* offsets changed */
    EXPECT_CALL(mTestObserver, onViewCreated("Group 10"_as)); /* offsets changed */
    EXPECT_CALL(mTestObserver, onViewCreated("15"_as));       /* new item */
    {
        auto write = state->ints.writeScope();
        write->removeAll(3);
        write->push_back(15);
    }
    validateOrder();
    EXPECT_FALSE(By::text("3").one());
    EXPECT_TRUE(By::text("15").one());

    /* update & reorder to new group */
    EXPECT_CALL(mTestObserver, onViewCreated("Group 0"_as));  /* offsets changed */
    EXPECT_CALL(mTestObserver, onViewCreated("Group 10"_as)); /* offsets changed */
    EXPECT_CALL(mTestObserver, onViewCreated("Group 20"_as)); /* new group */
    EXPECT_CALL(mTestObserver, onViewCreated("22"_as));       /* new group */
    {
        auto write = state->ints.writeScope();
        write->removeAll(4);
        write->push_back(22);
    }
    validateOrder();
    EXPECT_FALSE(By::text("4").one());
    EXPECT_TRUE(By::text("Group 20").one());
    EXPECT_TRUE(By::text("22").one());

    /* remove first element from the group */
    EXPECT_CALL(mTestObserver, onViewCreated("Group 0"_as));  /* offsets changed */
    EXPECT_CALL(mTestObserver, onViewCreated("Group 10"_as)); /* offsets changed */
    EXPECT_CALL(mTestObserver, onViewCreated("Group 20"_as)); /* offsets changed */
    {
        state->ints.writeScope()->removeAll(1);
    }
    validateOrder();

    /* update value, preserving order */
    EXPECT_FALSE(By::text("2").one());
    EXPECT_CALL(mTestObserver, onViewCreated("Group 0"_as));  /* contents changed */
    EXPECT_CALL(mTestObserver, onViewCreated("2"_as));  /* contents changed */
    {
        EXPECT_EQ((*state->ints)[0], 5);
        (*state->ints.writeScope())[0] = 2;
    }
    validateOrder();
    EXPECT_TRUE(By::text("2").one());
    saveScreenshot("");
    EXPECT_EQ(cache<AForEachUI<int>>().size(), 0);
}
