#include <gtest/gtest.h>
#include <AUI/Model/AListModel.h>
#include <AUI/View/AListView.h>

TEST(AListView, MeasureUpdatesWhenModelSizeChanges) {
    auto model = _new<AListModel<AString>>();
    (*model) << "First item";

    AListView listView(model);

    const auto oneItem = listView.measure(AConstraints::fixedWidth(0));

    (*model) << "Second item";
    const auto twoItems = listView.measure(AConstraints::fixedWidth(0));

    model->pop_back();
    const auto oneItemAgain = listView.measure(AConstraints::fixedWidth(0));

    EXPECT_EQ(oneItem.x, 0);
    EXPECT_EQ(twoItems.x, 0);
    EXPECT_EQ(oneItemAgain.x, 0);
    EXPECT_GT(twoItems.y, oneItem.y);
    EXPECT_EQ(oneItemAgain.y, oneItem.y);
}
