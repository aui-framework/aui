#include <gtest/gtest.h>
#include <AUI/Common/FixedSizeCache.h>

TEST(FixedSizeCache, KeepsItemsSortedAndDoesNotOverwriteExistingKeys) {
    FixedSizeCache<int, int, 4> cache;

    EXPECT_TRUE(cache.emplace(5, 50).second);
    EXPECT_TRUE(cache.emplace(1, 10).second);
    EXPECT_TRUE(cache.emplace(3, 30).second);

    auto it = cache.begin();
    ASSERT_NE(it, cache.end());
    EXPECT_EQ(it++->first, 1);
    ASSERT_NE(it, cache.end());
    EXPECT_EQ(it++->first, 3);
    ASSERT_NE(it, cache.end());
    EXPECT_EQ(it++->first, 5);
    EXPECT_EQ(it, cache.end());

    const auto [existing, inserted] = cache.emplace(3, 999);
    EXPECT_FALSE(inserted);
    ASSERT_NE(existing, cache.end());
    EXPECT_EQ(existing->second, 30);
}

TEST(FixedSizeCache, ClearsCacheOnOverflow) {
    FixedSizeCache<int, int, 2> cache;

    cache.emplace(2, 20);
    cache.emplace(1, 10);
    const auto [it, inserted] = cache.emplace(3, 30);

    EXPECT_TRUE(inserted);
    ASSERT_NE(it, cache.end());
    EXPECT_EQ(it->first, 3);
    EXPECT_EQ(it->second, 30);
    EXPECT_EQ(cache.size(), 1);
    EXPECT_EQ(cache.find(1), cache.end());
    EXPECT_EQ(cache.find(2), cache.end());
}
