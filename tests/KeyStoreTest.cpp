#include <gtest/gtest.h>

#include <sys/stat.h>

#include "Remote/KeyStore.h"
#include "TestUtil.h"

using namespace aui_remote;

namespace {
AppInfo app(std::uint32_t pid, AString startedAt) {
    AppInfo a;
    a.pid = pid;
    a.port = 1;
    a.startedAt = std::move(startedAt);
    return a;
}
}   // namespace

TEST(KeyStoreTest, GeneratedKeyIsRandomHex) {
    auto a = KeyStore::generateKey();
    auto b = KeyStore::generateKey();
    EXPECT_EQ(a.length(), 32u);
    EXPECT_NE(a, b);
    for (auto c : a) {
        EXPECT_TRUE(std::isxdigit(int(c))) << a;
    }
}

TEST(KeyStoreTest, StoreAndLoad) {
    test::TempDir dir;
    KeyStore store(dir / "clients");
    auto a = app(100, "2026-10-25T10:00:00Z");
    EXPECT_FALSE(store.load(a));
    store.store(a, "abcdef");
    EXPECT_EQ(store.load(a), "abcdef");
    EXPECT_EQ(store.pathFor(a).filename(), "100-2026_10_25T10_00_00Z.key");
}

TEST(KeyStoreTest, GeneratedKeyRoundTrips) {
    test::TempDir dir;
    KeyStore store(dir.path());
    const auto key = KeyStore::generateKey();
    store.store(app(7, "t"), key);
    EXPECT_EQ(store.load(app(7, "t")), key);
}

TEST(KeyStoreTest, KeyBelongsToTheProcessNotThePid) {
    test::TempDir dir;
    KeyStore store(dir.path());
    store.store(app(100, "t1"), "key1");
    EXPECT_FALSE(store.load(app(100, "t2")));
    EXPECT_EQ(store.load(app(100, "t1")), "key1");
}

TEST(KeyStoreTest, OverwriteReplacesTheKey) {
    test::TempDir dir;
    KeyStore store(dir.path());
    store.store(app(1, "t"), "long-old-key-value");
    store.store(app(1, "t"), "new");
    EXPECT_EQ(store.load(app(1, "t")), "new");
}

TEST(KeyStoreTest, FileIsPrivate) {
    test::TempDir dir;
    KeyStore store(dir.path());
    store.store(app(1, "t"), "secret");
    struct stat st {};
    ASSERT_EQ(::stat(store.pathFor(app(1, "t")).toStdString().c_str(), &st), 0);
    EXPECT_EQ(st.st_mode & 0777, 0600);
}

TEST(KeyStoreTest, PruneRemovesKeysOfGoneApps) {
    test::TempDir dir;
    KeyStore store(dir.path());
    store.store(app(1, "a"), "k1");
    store.store(app(2, "b"), "k2");
    store.pruneExcept({ app(2, "b") });
    EXPECT_FALSE(store.load(app(1, "a")));
    EXPECT_TRUE(store.load(app(2, "b")));
}

TEST(KeyStoreTest, Remove) {
    test::TempDir dir;
    KeyStore store(dir.path());
    store.store(app(1, "a"), "k1");
    store.remove(app(1, "a"));
    EXPECT_FALSE(store.load(app(1, "a")));
    store.remove(app(1, "a"));   // not an error
}
